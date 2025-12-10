#pragma once

#include <vector>
#include <memory>

#include <eventbus.h>
#include <camera.h>

#include "Entity.h"
#include "components/Mesh.h"

class Scene {
public:
    Scene(EventBus& bus);
    ~Scene() = default;

    // parts of a scene
    std::vector<std::shared_ptr<Entity>> entities;
    Camera camera;

    // event system
    EventBus& bus;

    // entity management
    void addEntity(std::shared_ptr<Entity> entity);
    void removeEntity(std::shared_ptr<Entity> entity);

    void update(float deltaTime);
    void render();

private:
    // single instance
    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;
};