#include "Scene.h"
#include "lights/DirectionalLight.h"

// TODO: REFACTOR THIS CONSTRUCTOR
Scene::Scene(EventBus& bus)
    : bus(bus), camera(), skybox(std::make_unique<Skybox>()) {

}

void Scene::setSkybox(const std::string& hdriPath) {
	//if (!skybox) {
	//	skybox = std::make_unique<Skybox>();
	//}
	skybox->load(hdriPath);
}

void Scene::update(float deltaTime) {
	// update entities
	for (auto& entity : objects) {
		entity->update(deltaTime);
	}

	// DEBUG LIGHT ROTATION THINGY!!!
	static float t = 0;
	t += deltaTime * 0.5f;

	float step = deltaTime * 0.5f;
	for (auto& light : lights) {
		if (auto dir = std::dynamic_pointer_cast<DirectionalLight>(light)) {
			glm::mat3 rotY = glm::rotate(glm::mat4(1.0f), step, glm::vec3(0, 1, 0));
			dir->direction = glm::normalize(rotY * dir->direction);
		}
	}
}

void Scene::addObject(std::shared_ptr<Object> entity) {
	if (entity) {
		objects.push_back(std::move(entity));
	}
}

void Scene::addObjects(std::vector<std::shared_ptr<Object>> objects) {
	for (auto object : objects) {
		addObject(object);
	}
}

void Scene::removeObject(std::shared_ptr<Object> entity) {
	auto it = std::remove(objects.begin(), objects.end(), entity);
	if (it != objects.end()) {
		objects.erase(it, objects.end());
	}
}

void Scene::addLight(std::shared_ptr<Light> light) {
	if (light) {
		lights.push_back(std::move(light));
	}
}

void Scene::removeLight(std::shared_ptr<Light> light) {
	auto it = std::remove(lights.begin(), lights.end(), light);
	if (it != lights.end()) {
		lights.erase(it, lights.end());
	}
}