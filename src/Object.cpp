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
    }
    else {
        logger.error("SHADER NOT FOUND");
        std::exit(EXIT_FAILURE);
    }

    // render entity
    // TODO: iterate through meshes and materials

    // find a better way to do this with safety checks
    //for (size_t i = 0; i < meshes.size(); i++) {
    //    if (i < materials.size()) {
    //        // apply materials
    //        materials[i]->apply(*shader);
    //    }

    //    // render meshes
    //    meshes[i]->render();
    //}

    // temporary solution :) no materials
    for (auto& mesh : meshes) {
        mesh->render();
    }
}