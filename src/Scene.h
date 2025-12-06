#pragma once

#include <vector>
#include <memory>

#include "Entity.h"
#include "eventbus.h"

class Scene {
public:
    Scene(EventBus& bus);
    ~Scene() = default;

    // single instance
    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;
    Scene(Scene&&) = delete;
    Scene& operator=(Scene&&) = delete;

    void addEntity(std::shared_ptr<Entity> entity);
    void removeEntity(std::shared_ptr<Entity> entity);

    void update(float deltaTime);
    void render();

private:
    std::vector<std::shared_ptr<Entity>> entities;
    EventBus& bus;
};