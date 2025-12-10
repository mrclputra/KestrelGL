#include "Scene.h"

Scene::Scene(EventBus& bus)
    : bus(bus), camera(6.0f, 30.0f, 36.0f) {
	logger.info("scene created...");
	camera.update(); // init
}
void Scene::update(float deltaTime) {
	// update camera
	camera.update();

	// update entities
	for (auto& entity : entities) {
		entity->update(deltaTime);
	}
}

void Scene::render() {
	for (auto& entity : entities) {
		// TODO: is it possible to not pass the matrices in the render function? 
        //  in other words, get it from camera object itself
		entity->render(camera.getViewMatrix(), camera.getProjectionMatrix());
	}
}

void Scene::addEntity(std::shared_ptr<Entity> entity) {
	if (entity) {
		entities.push_back(std::move(entity));
	}
}

void Scene::removeEntity(std::shared_ptr<Entity> entity) {
	auto it = std::remove(entities.begin(), entities.end(), entity);
	if (it != entities.end()) {
		entities.erase(it, entities.end());
	}
}
