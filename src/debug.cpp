#include "debug.h"
#include "ModelLoader.h"

#include <string>
#include <shader.h>

void debugScene(Scene& scene) {
	logger.info("constructing debug scene");
	logger.info("MAKING STUFF");

	auto shader = std::make_shared<Shader>(SHADER_DIR "model.vert", SHADER_DIR "model.frag");

	const int cX = 3;
	const int cY = 3;
	const int cZ = 3;
	const float spacing = 2.0f;

	for (int x = 0; x < cX; ++x) {
		for (int y = 0; y < cY; ++y) {
			for (int z = 0; z < cZ; ++z) {
				auto entity = ModelLoader::load("assets/models/sphere.obj");
				entity->shader = shader;
				if (!entity) continue;

				// set position
				entity->transform.position = glm::vec3(
					x * spacing - ((cX - 1.0f) * spacing * 0.5f),
					y * spacing - ((cY - 1.0f) * spacing * 0.5f),
					z * spacing - ((cZ - 1.0f) * spacing * 0.5f)
				);

				scene.addEntity(entity);
			}
		}
	}
}