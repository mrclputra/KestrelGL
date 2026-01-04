#include "Renderer.h"

void Renderer::init(const Scene& scene) {
    try {
        // todo: check where each shader is initialized/called from, case-by-case

        // set the depth shader
        // this is used for shadows
        depthShader = new Shader(SHADER_DIR "depth.vert", SHADER_DIR "depth.frag");
    }
    catch (const ShaderException& e) {
        logger.error("Failed to initialize shader: " + std::string(e.what()));
    }
}

void Renderer::render(const Scene& scene) {
    renderLightPass(scene);
    scene.skybox->m_SkyboxShader->checkHotReload();

	for (const auto& objPtr : scene.objects) {
		renderObject(scene, *objPtr);
	}

    if (scene.skybox) {
        scene.skybox->draw(scene.camera.getViewMatrix(),
            scene.camera.getProjectionMatrix(),
            scene.camera.position);
    }
    else {
        logger.error("NO SKYBOX FOR RENDERER");
    }
}

void Renderer::renderLightPass(const Scene& scene) {
    for (auto light : scene.lights) {
        if (auto dirLight = std::dynamic_pointer_cast<DirectionalLight>(light)) {

            // maybe should move this to an update function in the scene class?
            // I just put it here for now cuz it's more related to rendering
            dirLight->updateLightSpaceMatrix();
        
            // NOTE: whenever framebuffers are used again in the future, 
            // try to use the pattern below for viewport handling:

            // backup current viewport
            GLint viewport[4];
            glGetIntegerv(GL_VIEWPORT, viewport);

            // bind shadow framebuffer
            glBindFramebuffer(GL_FRAMEBUFFER, dirLight->depthMapFBO);
            glViewport(0, 0, dirLight->SHADOW_WIDTH, dirLight->SHADOW_HEIGHT);
            glClear(GL_DEPTH_BUFFER_BIT);

            // render scene from the light
            depthShader->use();
            depthShader->setMat4("lightSpaceMatrix", dirLight->lightSpaceMatrix);

            for (auto& objPtr : scene.objects) {
                // here we need to use a depth shader that only retrieves depth information
                depthShader->setMat4("model", objPtr->transform.getModelMatrix());
                for (auto& mesh : objPtr->meshes)
                    mesh->render();
            }

            // unbind FBO, restore viewport
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
        }
    }
}

void Renderer::renderObject(const Scene& scene, const Object& object) {
    if (!object.shader || object.shader->ID == 0) return;

	Shader& shader = *object.shader;
	
	shader.checkHotReload();

    try {
        shader.use();
    }
    catch (const ShaderException& e) {
        return;
    }

    shader.setInt("mode", renderMode);
    shader.setBool("normalEnabled", isNormalEnabled);

	shader.setMat4("model", object.transform.getModelMatrix());
	shader.setMat4("view", scene.camera.getViewMatrix());
	shader.setMat4("projection", scene.camera.getProjectionMatrix());
	shader.setVec3("viewPos", scene.camera.position);

    // base pbr parameters (non-texture)
    //  use 'p_' convention for overrideable parameters like this
    shader.setVec3("p_albedo", object.material->albedo);
    shader.setFloat("p_metalness", object.material->metalness);
    shader.setFloat("p_roughness", object.material->roughness);

    // shader texture flags
    shader.setBool("hasAlbedoMap", false);
    shader.setBool("hasNormalMap", false);
    shader.setBool("hasMetRoughMap", false);
    shader.setBool("hasAOMap", false);

    // lights
	uploadLights(scene, shader);

    // shadows
    int shadowMapStartSlot = 10;
    int dirLightCount = 0;
    for (auto& light : scene.lights) {
        if (auto dirLight = std::dynamic_pointer_cast<DirectionalLight>(light)) {
            if (dirLightCount >= 10) break;

            // upload transformation matrix
            std::string matrixName = "lightSpaceMatrices[" + std::to_string(dirLightCount) + "]";
            shader.setMat4(matrixName, dirLight->lightSpaceMatrix);

            // bind depth
            glActiveTexture(GL_TEXTURE0 + shadowMapStartSlot + dirLightCount);
            glBindTexture(GL_TEXTURE_2D, dirLight->depthMap);

            // tell shader which slot this index uses
            std::string samplerName = "shadowMaps[" + std::to_string(dirLightCount) + "]";
            shader.setInt(samplerName, shadowMapStartSlot + dirLightCount);

            dirLightCount++;
        }
    }

    // skybox and IBL data
    if (scene.skybox) {
        // irradiance
        if (!scene.skybox->shCoefficients.empty()) {
            glUniform3fv(glGetUniformLocation(shader.ID, "shCoefficients"), 9, &scene.skybox->shCoefficients[0].x);
        }

        // prefilter map
        glActiveTexture(GL_TEXTURE9);
        glBindTexture(GL_TEXTURE_CUBE_MAP, scene.skybox->m_PrefilterMap);
        shader.setInt("prefilterMap", 9);

        // todo: brdf lut map
    }

    // TODO: refactor the textures binding system for multi-mesh objects
    // mesh-specific texture binding
    for (const auto& mesh : object.meshes) {
        for (int texIdx : mesh->textureIndices) {
            const auto& tex = object.material->textures[texIdx];

            switch (tex->type) {
                case Texture::Type::ALBEDO: 
                    if (object.material->useAlbedoTexture) {
                        shader.setBool("hasAlbedoMap", true);
                        shader.setInt("albedoMap", 0);
                        tex->bind(0);
                    }
                    break;
                case Texture::Type::NORMAL: 
                    shader.setBool("hasNormalMap", true);
                    shader.setInt("normalMap", 1);
                    tex->bind(1);
                    break;
                case Texture::Type::METALLIC_ROUGHNESS: 
                    if (object.material->useMetRoughTexture) {
                        shader.setBool("hasMetRoughMap", true);
                        shader.setInt("metRoughMap", 2);
                        tex->bind(2);
                    }
                    break;
                case Texture::Type::OCCLUSION: 
                    shader.setBool("hasAOMap", true);
                    shader.setInt("aoMap", 3);
                    tex->bind(3);
                    break;
            }
        }

        mesh->render();
    }
}

void Renderer::uploadLights(const Scene& scene, Shader& shader) {
    constexpr int MAX_LIGHTS = 8; // this is arbitrary

    int dirCount = 0;
    int pointCount = 0;
    int spotCount = 0;

    for (auto& light : scene.lights) {
        if (auto dir = std::dynamic_pointer_cast<DirectionalLight>(light)) {
            if (dirCount >= MAX_LIGHTS) continue;

            std::string base = "dirLights[" + std::to_string(dirCount) + "]";
            shader.setVec3(base + ".direction", dir->direction);
            shader.setVec3(base + ".color", dir->color);

            dirCount++;
        }
        else if (auto point = std::dynamic_pointer_cast<PointLight>(light)) {
            if (pointCount >= MAX_LIGHTS) continue;

            std::string base = "pointLights[" + std::to_string(pointCount) + "]";
            shader.setVec3(base + ".position", point->transform.position);
            shader.setVec3(base + ".color", point->color);
            shader.setFloat(base + ".radius", point->radius);

            pointCount++;
        }
        // TODO: spot lights
    }

    shader.setInt("numDirLights", dirCount);
    shader.setInt("numPointLights", pointCount);
    shader.setInt("numSpotLights", spotCount);
}