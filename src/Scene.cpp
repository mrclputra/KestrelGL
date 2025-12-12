#include "Scene.h"

Scene::Scene(EventBus& bus)
    : bus(bus), camera(6.0f, 0.0f, 0.0f) {
	logger.info("scene created...");
	camera.update(); // init
}
void Scene::update(float deltaTime) {
	// update entities
	for (auto& entity : entities) {
		entity->update(deltaTime);
	}
}

void Scene::render() {
	// fill pass
	for (auto& entity : entities) {
		// TODO: is it possible to not pass the matrices in the render function? 
        //  in other words, get it from camera object itself
		entity->shader->setVec3("albedo", glm::vec3(0.98f));
		entity->render(camera.getViewMatrix(), camera.getProjectionMatrix());
	}

	// wireframe pass
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	//glEnable(GL_POLYGON_OFFSET_LINE);
	//glPolygonOffset(-1.0f, 1.0f);
	//glLineWidth(1.2f);
	//for (auto& entity : entities) {
	//	entity->shader->setVec3("albedo", glm::vec3(0.0f));
	//	entity->render(camera.getViewMatrix(), camera.getProjectionMatrix());
	//}
	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void Scene::addEntity(std::shared_ptr<Object> entity) {
	if (entity) {
		entities.push_back(std::move(entity));
	}
}

void Scene::removeEntity(std::shared_ptr<Object> entity) {
	auto it = std::remove(entities.begin(), entities.end(), entity);
	if (it != entities.end()) {
		entities.erase(it, entities.end());
	}
}
