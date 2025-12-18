#pragma once

#include <vector>
#include <memory>

#include <eventbus.h>

#include "Object.h"
#include "Camera.h"
#include "lights/Light.h"
#include "components/Mesh.h"

class Scene {
public:
    Scene(EventBus& bus);
    ~Scene() = default;

    // parts of a scene
    std::vector<std::shared_ptr<Object>> objects;
    std::vector<std::shared_ptr<Light>> lights;
    Camera camera;

    // event system
    EventBus& bus;

    // object management
    void addObject(std::shared_ptr<Object> object);
    void removeObject(std::shared_ptr<Object> object);

    // lights management
    void addLight(std::shared_ptr<Light> light);
    void removeLight(std::shared_ptr<Light> light);

    void update(float deltaTime);

private:
    // single instance
    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;
};