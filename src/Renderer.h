// ALL rendering operations should be done in this class
// The Renderer() class reads the Scene tree and performs rendering operations
#pragma once

#include <vector>
#include "Scene.h"

class Renderer {
public:
	void init(const Scene& scene);
	void render(const Scene& scene);

private:
	struct DrawCommand {
		const Mesh* mesh;
		const Material* material;
		glm::mat4 modelMatrix;
		float distance; // need to find a better approach for transparency
	};

	std::vector<DrawCommand> commands;

	void collectDrawCommands(const Scene& scene);
	void executeBatched(const Scene& scene);
	void renderSkybox(const Scene& scene);
};