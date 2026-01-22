#pragma once

#include <vector>
#include <memory>

#include <eventbus.h>

#include "Camera.h"
#include "Skybox.h"
#include "lights/Light.h"
#include "components/Object.h"
#include "components/Mesh.h"
#include "components/Texture.h"

class Scene {
public:
    Scene();
    ~Scene() = default;

    // parts of a scene
    std::vector<std::shared_ptr<Object>> objects;
    std::vector<std::shared_ptr<Texture>> textures;
    std::vector<std::shared_ptr<Light>> lights;
    Camera camera;

    // skybox
    std::unique_ptr<Skybox> skybox;

    // event system
    //EventBus& bus;

    // skybox management
    void setSkybox(const std::string& hdriPath);

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