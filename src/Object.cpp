#include "Object.h"

// constructor
Object::Object(const std::string& name) {
    this->name = name;
}

void Object::update(float deltaTime) {
    // DEBUG rotation
    //transform.rotate(glm::vec3(.0f, .5f, .0f) * deltaTime * 1.7f);
    shader->checkHotReload();
}