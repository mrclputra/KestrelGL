#include "Renderer.h"

// todo: rename this init function to something else? 
// as it depends on current scene data which may change when i add a loading system in the future
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

    // initialize shadow maps
    int numDirLights = 0;
    for (auto& light : scene.lights) {
        if (std::dynamic_pointer_cast<DirectionalLight>(light)) {
            numDirLights++;
        }
    }
    if (numDirLights > 0) {
        // creates the 2D array texture
        glGenFramebuffers(1, &shadowArrayFBO);

        glGenTextures(1, &shadowArrayTexture);
        glBindTexture(GL_TEXTURE_2D_ARRAY, shadowArrayTexture);
        glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT,
            SHADOW_WIDTH, SHADOW_HEIGHT, numDirLights, 0,
            GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

        float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, borderColor);

        glBindFramebuffer(GL_FRAMEBUFFER, shadowArrayFBO);
        glFramebufferTexture3D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_ARRAY, shadowArrayTexture, 0, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // we will attach layers individually on the render loop
        // for now, we just allocate the memory

        // assign layer indices to each light
        int layerIdx = 0;
        for (auto& light : scene.lights) {
            if (auto dirLight = std::dynamic_pointer_cast<DirectionalLight>(light)) {
                dirLight->shadowArrayLayer = layerIdx++;
            }
        }
    }

    // generate brdf map
    //brdfLUT = generateBRDFLUT();
}

void Renderer::render(const Scene& scene) {
    // render skybox
    renderSkybox(scene);

    // render shadow maps
    //renderShadowPass(scene);

    // render objects
    drawOrder.clear();
    for (const std::shared_ptr<Object> &object : scene.objects) {
        // sort objects by distance from camera
        // this is so wecan properly implement blending
        float distance = glm::length(scene.camera.position - object->transform.position);
        drawOrder.insert(std::make_pair(distance, object));
    }
    // render the objects
    for (const auto &pair : drawOrder) {
        renderObject(scene, *pair.second);
    }
}

void Renderer::renderObject(const Scene& scene, const Object& object) {
    if (!object.shader || object.shader->ID == 0) return;

    Shader& shader = *object.shader;

    shader.checkHotReload();
    shader.use();

    shader.setInt("mode", renderMode);

    // object transformations
    shader.setMat4("model", object.transform.getModelMatrix());
    
    // camera
    shader.setMat4("view", scene.camera.getViewMatrix());
    shader.setMat4("projection", scene.camera.getProjectionMatrix());
    shader.setVec3("viewPos", scene.camera.position);

    // base pbr parameters (non-texture)
    shader.setVec4("p_albedo", object.material->albedo);
    shader.setFloat("p_metalness", object.material->metalness);
    shader.setFloat("p_roughness", object.material->roughness);

    // shader manual texture flags
    shader.setBool("useAlbedoMap", object.material->useAlbedoMap);
    shader.setBool("useNormalMap", object.material->useNormalMap);
    shader.setBool("useMetRoughMap", object.material->useMetRoughMap);
    shader.setBool("useAOMap", object.material->useAOMap);
    shader.setBool("useEmissionMap", object.material->useEmissionMap);

    // lights
    //int dirCount = 0;
    //for (auto& light : scene.lights) {
    //    if (auto dir = std::dynamic_pointer_cast<DirectionalLight>(light)) {
    //        if (dirCount >= 8) continue; // max lights

    //        std::string base = "dirLights[" + std::to_string(dirCount) + "]";
    //        shader.setVec3(base + ".direction", dir->direction);
    //        shader.setVec3(base + ".color", dir->color);

    //        dirCount++;
    //    }
    //    // TODO: point lights
    //    // TODO: spot lights
    //}
    //shader.setInt("numDirLights", dirCount);

    // shadows
    //glActiveTexture(GL_TEXTURE8);
    //glBindTexture(GL_TEXTURE_2D_ARRAY, shadowArrayTexture);
    //shader.setInt("shadowMaps", 8);

    //int dirLightCount = 0;
    //for (auto& light : scene.lights) {
    //    if (auto dirLight = std::dynamic_pointer_cast<DirectionalLight>(light)) {
    //        if (dirLightCount >= 10) break;

    //        // upload transformation matrix
    //        std::string matrixName = "lightSpaceMatrices[" + std::to_string(dirLightCount) + "]";
    //        shader.setMat4(matrixName, dirLight->lightSpaceMatrix);

    //        dirLightCount++;
    //    }
    //}

    // skybox and IBL data
    //if (scene.skybox) {
    //    // irradiance
    //    if (!scene.skybox->shCoefficients.empty()) {
    //        glUniform3fv(glGetUniformLocation(shader.ID, "shCoefficients"), 9, &scene.skybox->shCoefficients[0].x);
    //    }

    //    // prefilter map
    //    glActiveTexture(GL_TEXTURE10);
    //    glBindTexture(GL_TEXTURE_CUBE_MAP, scene.skybox->m_PrefilterMap);
    //    shader.setInt("prefilterMap", 10);

    //    // brdf lut map
    //    glActiveTexture(GL_TEXTURE11);
    //    glBindTexture(GL_TEXTURE_2D, brdfLUT);
    //    shader.setInt("brdfLUT", 11);
    //}

    // load textures
    for (const auto& mesh : object.meshes) {
        // I believe this is causing a bottleneck?
        //  as we need to iterate through every mesh...
        for (int texIdx : mesh->textureIndices) {
            const auto& tex = object.material->textures[texIdx];

            switch (tex->type) {
            case Texture::Type::ALBEDO:
                shader.setBool("hasAlbedoMap", true);
                shader.setInt("albedoMap", 0);
                tex->bind(0);
                break;
            case Texture::Type::NORMAL:
                shader.setBool("hasNormalMap", true);
                shader.setInt("normalMap", 1);
                tex->bind(1);
                break;
                // uncomment when needed:
                //case Texture::Type::METALLIC_ROUGHNESS:
                //    shader.setBool("hasMetRoughMap", true);
                //    shader.setInt("metRoughMap", 2);
                //    tex->bind(2);
                //    break;
                //case Texture::Type::OCCLUSION:
                //    shader.setBool("hasAOMap", true);
                //    shader.setInt("aoMap", 3);
                //    tex->bind(3);
                //    break;
                //case Texture::Type::EMISSION:
                //    shader.setBool("hasEmissionMap", true);
                //    shader.setInt("emissionMap", 4);
                //    tex->bind(4);
            }
        }

        mesh->render();
    }

    // 
    //for (const auto& tex : object.material->textures) {
    //    switch (tex->type) {
    //    case Texture::Type::ALBEDO:
    //        shader.setBool("hasAlbedoMap", true);
    //        shader.setInt("albedoMap", 0);
    //        tex->bind(0);
    //        break;
    //    case Texture::Type::NORMAL:
    //        shader.setBool("hasNormalMap", true);
    //        shader.setInt("normalMap", 1);
    //        tex->bind(1);
    //        break;
    //        //case Texture::Type::METALLIC_ROUGHNESS:
    //        //    shader.setBool("hasMetRoughMap", true);
    //        //    shader.setInt("metRoughMap", 2);
    //        //    tex->bind(2);
    //        //    break;
    //        //case Texture::Type::OCCLUSION:
    //        //    shader.setBool("hasAOMap", true);
    //        //    shader.setInt("aoMap", 3);
    //        //    tex->bind(3);
    //        //    break;
    //        //case Texture::Type::EMISSION:
    //        //    shader.setBool("hasEmissionMap", true);
    //        //    shader.setInt("emissionMap", 4);
    //        //    tex->bind(4);
    //    }
    //}

    //for (const auto& mesh : object.meshes) {
    //    mesh->render();
    //}
}

void Renderer::renderShadowPass(const Scene& scene) {
    for (auto light : scene.lights) {
        if (auto dirLight = std::dynamic_pointer_cast<DirectionalLight>(light)) {

            // maybe should move this to an update function in the scene class?
            // I just put it here for now cuz it's more related to rendering
            dirLight->updateLightSpaceMatrix();
        
            // NOTE: whenever framebuffers are used again in the future, 
            // try to use the pattern below for viewport handling:

            // todo: see if there is a better way to do viewport handling?

            // backup current viewport
            GLint viewport[4];
            glGetIntegerv(GL_VIEWPORT, viewport);

            // bind shadow framebuffer
            glBindFramebuffer(GL_FRAMEBUFFER, shadowArrayFBO);
            glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                shadowArrayTexture, 0, dirLight->shadowArrayLayer);

            glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
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

void Renderer::renderSkybox(const Scene& scene) {
    scene.skybox->m_SkyboxShader->checkHotReload();
    if (scene.skybox) {
        scene.skybox->draw(scene.camera.getViewMatrix(),
            scene.camera.getProjectionMatrix(),
            scene.camera.position);
    }
    else {
        logger.error("NO SKYBOX FOR RENDERER");
    }
}

unsigned int Renderer::generateBRDFLUT() {
    glDisable(GL_DEPTH_TEST);

    unsigned int brdfLUTTexture;
    glGenTextures(1, &brdfLUTTexture);

    // allocate storage
    glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // framebuffer to render the LUT on
    unsigned int captureFBO, captureRBO;
    glGenFramebuffers(1, &captureFBO);
    glGenRenderbuffers(1, &captureRBO);
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUTTexture, 0);

    // backup viewport
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    // run shader
    glViewport(0, 0, 512, 512);
    Shader brdfShader(SHADER_DIR "brdf.vert", SHADER_DIR "brdf.frag");
    brdfShader.use();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    renderQuad();

    // restore
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

    // cleanup
    glDeleteFramebuffers(1, &captureFBO);
    glDeleteRenderbuffers(1, &captureRBO);

    glEnable(GL_DEPTH_TEST);

    logger.info("generated BRDF lookup table");

    return brdfLUTTexture;
}

// UTILITY
void Renderer::renderQuad() {
    static unsigned int quadVAO = 0;
    static unsigned int quadVBO;
    if (quadVAO == 0) {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };

        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}