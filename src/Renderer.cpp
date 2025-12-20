#include "Renderer.h"

void Renderer::init(const Scene& scene) {
    // set the depth shader
	depthShader = new Shader(SHADER_DIR "depth.vert", SHADER_DIR "depth.frag");

    // here is the question; do we need a separate shader instance for every light source?
}

void Renderer::render(const Scene& scene) {
    renderLightPass(scene);

	for (const auto& objPtr : scene.objects) {
		renderObject(scene, *objPtr);
	}
}

void Renderer::renderLightPass(const Scene& scene) {
    for (auto light : scene.lights) {
        if (auto dirLight = std::dynamic_pointer_cast<DirectionalLight>(light)) {

            // maybe should move this to an update function in the scene class?
            // I just put it here for now cuz it's more related to rendering
            dirLight->updateLightSpaceMatrix();
        
            // NOTE: whenever framebuffers are used again in the future, try to use the following pattern for viewport handling

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
	Shader& shader = *object.shader;
	
	shader.checkHotReload();
	shader.use();

	shader.setMat4("model", object.transform.getModelMatrix());
	shader.setMat4("view", scene.camera.getViewMatrix());
	shader.setMat4("projection", scene.camera.getProjectionMatrix());
	shader.setVec3("viewPos", scene.camera.position);

	uploadLights(scene, shader);

	// textures
	unsigned int slot = 0;
	for (const auto& tex : object.textures) {
		tex->bind(slot);
		switch (tex->type) {
			case Texture::Type::ALBEDO: shader.setInt("albedoMap", slot); break;
			case Texture::Type::NORMAL: shader.setInt("normalMap", slot); break;
			case Texture::Type::METALLIC_ROUGHNESS: shader.setInt("metallicRoughnessMap", slot); break;
			case Texture::Type::OCCLUSION: shader.setInt("aoMap", slot); break;
			case Texture::Type::EMISSION: shader.setInt("emissionMap", slot); break;
		}
		++slot;
	}

    // TODO: there is probably a better way to do this
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

	for (const auto& mesh : object.meshes) {
		mesh->render();
	}
}

void Renderer::uploadLights(const Scene& scene, Shader& shader) {
    constexpr int MAX_LIGHTS = 8;

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
    }

    shader.setInt("numDirLights", dirCount);
    shader.setInt("numPointLights", pointCount);
    shader.setInt("numSpotLights", spotCount);
}