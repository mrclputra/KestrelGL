#pragma once

#include <vector>
#include <memory>

#include <eventbus.h>
#include <camera.h>

#include "Entity.h"
#include "Mesh.h"

class Scene {
public:
    Scene(EventBus& bus);
    ~Scene() = default;

    // single instance
    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;

    Camera camera;

    void addEntity(std::unique_ptr<Entity> entity);
    void removeEntity(std::unique_ptr<Entity> entity);

    void update(float deltaTime);
    void render();

    // DEBUG
    void createDebug();

private:
    std::vector<std::unique_ptr<Entity>> entities;
    EventBus& bus;
};