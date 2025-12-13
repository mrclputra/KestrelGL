#include "Scene.h"
#include "lights/DirectionalLight.h"

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
}

void Scene::render() {
	// fill pass
	for (auto& entity : objects) {
		// light DEBUG, TODO: remove
		auto& debugLight = lights[0];
		if (auto light = std::dynamic_pointer_cast<DirectionalLight>(debugLight)) {
			entity->shader->setVec3("lightDir", light->direction);
		}

		// TODO: is it possible to not pass the matrices in the render function? 
        //  in other words, get it from camera object itself
		//entity->shader->setVec3("albedo", glm::vec3(0.98f));
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