#include "Scene.h"
#include "lights/DirectionalLight.h"
#include "lights/PointLight.h"

Scene::Scene(EventBus& bus)
    : bus(bus), camera(6.0f, 0.0f, 0.0f) {
	logger.info("scene created...");
	camera.update(); // init
}

void Scene::update(float deltaTime) {
	// update entities
	for (auto& entity : objects) {
		entity->update(deltaTime);
	}

	// DEBUG LIGHT ROTATION THINGY!!!
	static float t = 0;
	t += deltaTime * 0.5f;
	if (auto it = std::find_if(lights.begin(), lights.end(),
		[](auto& l) { return std::dynamic_pointer_cast<PointLight>(l); }); it != lights.end())
	{
		auto& p = std::dynamic_pointer_cast<PointLight>(*it)->transform.position;
		p.x = 4.0f * sin(t * glm::two_pi<float>());
	}
}

void Scene::addObject(std::shared_ptr<Object> entity) {
	if (entity) {
		objects.push_back(std::move(entity));
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