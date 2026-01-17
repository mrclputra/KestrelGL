#include "Scene.h"
#include "lights/DirectionalLight.h"

// TODO: REFACTOR THIS CONSTRUCTOR
Scene::Scene() : camera(), skybox(std::make_unique<Skybox>()) {}

void Scene::setSkybox(const std::string& hdriPath) {
	skybox->load(hdriPath);
}

void Scene::update(float deltaTime) {
	// update transforms
	for (auto& object : objects) {
		object->update(deltaTime);
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

void Scene::addObject(std::shared_ptr<Object> object) {
	objects.push_back(std::move(object));
}

void Scene::removeObject(std::shared_ptr<Object> object) {
	auto it = std::remove(objects.begin(), objects.end(), object);
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