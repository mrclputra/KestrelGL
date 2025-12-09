#include "Entity.h"

// constructor
Entity::Entity(const std::string& name, std::shared_ptr<Shader> shaderPtr) 
    : name(name), shader(shaderPtr) {

    if (!mesh) mesh = getDefaultCubeMesh();
    logger.info("entity created: " + name + ", " +
        "\tshader: " + std::to_string(reinterpret_cast<uintptr_t>(shaderPtr.get())) + ", " +
        "\tmesh: " + std::to_string(reinterpret_cast<uintptr_t>(mesh.get())));
}
Entity::~Entity() {}

void Entity::update(float deltaTime) {
    // DEBUG rotation
    rotation += glm::vec3(.5f, .5f, .5f) * deltaTime;
}

void Entity::render(const glm::mat4& view, const glm::mat4& projection) {
    if (!mesh) {
        logger.error(name + " missing mesh");
        return;
    }
    if (!shader) {
        logger.error(name + " missing shader");
        return;
    }

    shader->use();

    // TODO: bind textures if available

    shader->setMat4("model", getModelMatrix());
    shader->setMat4("view", view);
    shader->setMat4("projection", projection);

    shader->setVec3("color", glm::vec3(0.9f, 0.9f, 0.9f));

    // maybe not need to pass shaders if mesh doesnt modify shader
    mesh->render(*shader);

    // TODO: unbind textures
}

// relative transforms
void Entity::translate(const glm::vec3& delta) { position += delta; }
void Entity::rotate(const glm::vec3& delta) { rotation += delta; }
void Entity::rescale(const glm::vec3& factor) { scale *= factor; }

glm::mat4 Entity::getModelMatrix() const {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::rotate(model, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, scale);
    return model;
}

// DEBUG
std::shared_ptr<Mesh> Entity::getDefaultCubeMesh() {
    static std::shared_ptr<Mesh> defaultMesh = nullptr;

    if (!defaultMesh) {
        using Vertex = Mesh::Vertex;
        std::vector<Vertex> vertices = {
            {{-0.5f,-0.5f,-0.5f},{0,0,-1},{0,0,0},{0,0,0},{0,0}},
            {{0.5f,-0.5f,-0.5f},{0,0,-1},{0,0,0},{0,0,0},{1,0}},
            {{0.5f,0.5f,-0.5f},{0,0,-1},{0,0,0},{0,0,0},{1,1}},
            {{-0.5f,0.5f,-0.5f},{0,0,-1},{0,0,0},{0,0,0},{0,1}},
            {{-0.5f,-0.5f,0.5f},{0,0,1},{0,0,0},{0,0,0},{0,0}},
            {{0.5f,-0.5f,0.5f},{0,0,1},{0,0,0},{0,0,0},{1,0}},
            {{0.5f,0.5f,0.5f},{0,0,1},{0,0,0},{0,0,0},{1,1}},
            {{-0.5f,0.5f,0.5f},{0,0,1},{0,0,0},{0,0,0},{0,1}}
        };

        std::vector<unsigned int> indices = {
            0,1,2,2,3,0, 4,5,6,6,7,4,
            4,5,1,1,0,4, 6,7,3,3,2,6,
            5,6,2,2,1,5, 4,7,3,3,0,4
        };

        defaultMesh = std::make_shared<Mesh>(vertices, indices);
        defaultMesh->upload();
    }

    return defaultMesh;
}