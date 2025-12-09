#include "Scene.h"
#include "ModelLoader.h"

Scene::Scene(EventBus& bus)
    : bus(bus), camera(6.0f, 30.0f, 36.0f) {
    camera.update();
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

void Scene::update(float deltaTime) {
	camera.update(); // update camera

	for (auto& e : entities) {
		e->update(deltaTime);
	}
}

void Scene::render() {
	for (auto& e : entities) {
		// TODO: is it possible to not pass the matrices in the render function? 
        //  in other words, get it from camera object itself
		e->render(camera.getViewMatrix(), camera.getProjectionMatrix());
	}
}

// TO DELETE LATER
void Scene::createDebug() {
    // shared shader
    // thinking about making a manager for this
    auto shader = std::make_shared<Shader>(SHADER_DIR "/model.vert", SHADER_DIR "/model.frag");
    //auto sphere = ModelLoader::load("assets/models/sphere.obj", shader);
    //if (sphere) {
    //    addEntity(std::move(sphere));
    //}

    const int countX = 3;
    const int countY = 3;
    const int countZ = 3;
    const float spacing = 1.0f;

    for (int x = 0; x < countX; x++) {
        for (int y = 0; y < countY; y++) {
            for (int z = 0; z < countZ; z++) {

                // compute world position
                glm::vec3 pos(
                    (x - countX / 2) * spacing,
                    (y - countY / 2) * spacing,
                    (z - countZ / 2) * spacing
                );

                // entity name
                std::string name = "Spehre_" +
                    std::to_string(x) + "_" +
                    std::to_string(y) + "_" +
                    std::to_string(z);
                
                // make entity
                //auto entity = std::make_shared<Entity>(name, shader);
                auto entity = ModelLoader::load("assets/models/sphere.obj", shader);

                // move entity
                entity->translate(pos);
                entity->rescale(glm::vec3(0.5f));

                // add to scene
                addEntity(std::move(entity));
            }
        }
    }
}