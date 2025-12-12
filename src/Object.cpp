#include "Object.h"

// constructor
Object::Object(const std::string& name) {
    this->name = name;
}

void Object::update(float deltaTime) {
    // DEBUG rotation
    //transform.rotate(glm::vec3(.0f, .5f, .0f) * deltaTime * 1.7f);
}

void Object::render(const glm::mat4& view, const glm::mat4& projection) {
    if (shader->ID != 0) {
        shader->use();

        shader->setMat4("model", transform.getModelMatrix());
        shader->setMat4("view", view);
        shader->setMat4("projection", projection);

        // TODO: handle multiple textures of same type
        // bind textures
        unsigned int slot = 0;
        for (auto& tex : textures) {
            tex->bind(slot);
            switch (tex->type) {
                case Texture::Type::ALBEDO: shader->setInt("albedoMap", slot); break;
                case Texture::Type::NORMAL: shader->setInt("normalMap", slot); break;
                case Texture::Type::METALLIC_ROUGHNESS: shader->setInt("metallicRoughnessMap", slot); break;
                case Texture::Type::OCCLUSION: shader->setInt("aoMap", slot); break;
                case Texture::Type::EMISSION: shader->setInt("emissionMap", slot); break;
            }
            slot++;
        }
    }
    else {
        logger.error("SHADER NOT FOUND");
        std::exit(EXIT_FAILURE);
    }

    // render mesh
    for (auto& mesh : meshes) {
        mesh->render();
    }
}