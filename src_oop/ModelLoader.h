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

class ModelLoader {
public:
	static std::shared_ptr<Object> load(const std::string& path);

	static void processNode(aiNode* assimpNode, const aiScene* assimpScene, const std::string& directory, const glm::mat4 transform, Object& object);

	static std::shared_ptr<Mesh> processMesh(aiMesh* assimpMesh, const aiScene* assimpScene, const std::string& directory, const glm::mat4 transform, Object& object);

	static int loadTexture(const std::string& path, Texture::Type type, Object& object);

	static glm::mat4 aiMatrixtoGLM(const aiMatrix4x4& from);
};
