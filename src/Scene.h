#pragma once

#include <vector>
#include <memory>

#include "Entity.h"
#include "eventbus.h"

class Scene {
public:
    Scene();
    ~Scene();

    void addEntity(std::shared_ptr<Entity> entity);
    void removeEntity(std::shared_ptr<Entity> entity);

    void update(float deltaTime);
    void render();

    std::vector<std::shared_ptr<Entity>> entities;
    EventBus& bus;

private:
    void setupEventHandlers();

};