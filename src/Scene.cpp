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

	// DEBUG LIGHT ROTATION THING
	float s = glm::radians(45.0f) * deltaTime;
	auto& p = lights[3]->transform.position;
	std::tie(p.x, p.z) = std::make_pair(p.x * cos(s) - p.z * sin(s),
		p.x * sin(s) + p.z * cos(s));
}

void Scene::render() {
	// TEMPORARY, NEED REFACTORING
	struct ShaderLight {
		glm::vec3 position;
		glm::vec3 direction;
		glm::vec3 color;
		int type; // 0=directional, 1=point, 2=spot
	};

	// fill pass
	for (auto& object : objects) {
		// LIGHTS
		ShaderLight shaderLights[8];  // MAX_LIGHTS
		int activeLights = std::min<int>(lights.size(), 8);

		// light DEBUG, TODO: remove
		for (int i = 0; i < activeLights; ++i) {
			auto& light = lights[i];
			
			shaderLights[i].color = light->color;

			if (auto dirLight = std::dynamic_pointer_cast<DirectionalLight>(light)) {
				// directional lighting
				shaderLights[i].position = glm::vec3(0.0f);
				shaderLights[i].direction = dirLight->direction;
				shaderLights[i].type = 0;
			}
			else if (auto pointLight = std::dynamic_pointer_cast<PointLight>(light)) {
				// TODO: spot lighting
				shaderLights[i].position = pointLight->transform.position;
				shaderLights[i].type = 1;
			}
			else if (false) {
				// TODO: point lighting
			}

			// upload light to shader
			std::string base = "lights[" + std::to_string(i) + "]";
			object->shader->setVec3(base + ".position", shaderLights[i].position);
			object->shader->setVec3(base + ".direction", shaderLights[i].direction);
			object->shader->setVec3(base + ".color", shaderLights[i].color);
			object->shader->setInt(base + ".type", shaderLights[i].type);
		}

		object->shader->setInt("numLights", activeLights);

		// pass camera position to shader, we can calculate the vector later
		object->shader->setVec3("viewPos", camera.position);

		// TODO: is it possible to not pass the matrices in the render function? 
        //  in other words, get it from camera object itself

		// RENDER OBJECT
		object->render(camera.getViewMatrix(), camera.getProjectionMatrix());
	}

	// DEPRECATED, need a separate shader if i want to do this again
	//	
	// wireframe pass 
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	//glEnable(GL_POLYGON_OFFSET_LINE);
	//glPolygonOffset(-1.0f, 1.0f);
	//glLineWidth(1.2f);
	//for (auto& object : entities) {
	//	object->shader->setVec3("albedo", glm::vec3(0.0f));
	//	object->render(camera.getViewMatrix(), camera.getProjectionMatrix());
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