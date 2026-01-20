#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <string>
#include <memory>
#include <vector>
#include <numeric>

#include <logger.h>
#include "Scene.h"

namespace ModelLoader {
	std::vector<std::shared_ptr<Object>> loadAsObjects(
		const std::string& path, 
		std::vector<std::shared_ptr<Texture>>& textureCache,
		glm::vec3 scale = glm::vec3(1.0f)
	);
}
