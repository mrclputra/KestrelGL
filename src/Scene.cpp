#include "Scene.h"

Scene::Scene(EventBus& bus) 
	: bus(bus) { }

void Scene::addEntity(std::shared_ptr<Entity> entity) {
	if (entity) {
		entities.push_back(entity);
	}
}

void Scene::removeEntity(std::shared_ptr<Entity> entity) {
	auto it = std::remove(entities.begin(), entities.end(), entity);
	if (it != entities.end()) {
		entities.erase(it, entities.end());
	}
}

void Scene::update(float deltaTime) {
	for (auto& e : entities) {
		e->update(deltaTime);
	}
}

void Scene::render() {
	for (auto& e : entities) {
		e->render();
	}
}