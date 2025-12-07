#include "Scene.h"

Scene::Scene(EventBus& bus) 
	: bus(bus), camera(5.0f, 0.0f, 0.0f) { }

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
	camera.update(); // update camera

	for (auto& e : entities) {
		e->update(deltaTime);
	}
}

void Scene::render() {
	for (auto& e : entities) {
		// TODO: see if there is a better way to do this? 
		// TOOD: is it possible to not pass the matrices in the render function? 
		e->render(camera.getViewMatrix(), camera.getProjectionMatrix());
	}
}

// TO DELETE LATER
void Scene::createDebug() {

    const int countX = 3;   // number of cubes along X
    const int countY = 3;   // number of cubes along Y
    const int countZ = 3;   // number of cubes along Z

    const float spacing = 2.0f; // distance between cubes

    for (int x = 0; x < countX; x++) {
        for (int y = 0; y < countY; y++) {
            for (int z = 0; z < countZ; z++) {

                std::string name = "Cube_" +
                    std::to_string(x) + "_" +
                    std::to_string(y) + "_" +
                    std::to_string(z);

                auto cube = std::make_shared<Entity>(name);

                // give it a mesh
                cube->meshes.push_back(std::make_shared<Mesh>());

                // compute world position
                glm::vec3 pos(
                    (x - countX / 2) * spacing,
                    (y - countY / 2) * spacing,
                    (z - countZ / 2) * spacing
                );

                cube->translate(pos);

                addEntity(cube);
            }
        }
    }
}