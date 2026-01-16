#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <string>
#include <memory>
#include <vector>
#include <numeric>

#include <logger.h>

#include "components/Object.h"

// this should return a list of objects
class ModelLoader {
public:
	// todo: allow for custom shaders
	static std::vector<std::shared_ptr<Object>> load(const std::string& path);
	
	static void processNode(aiNode* assimpNode, const aiScene* assimpScene, const std::string& directory, const glm::mat4& parentTransform, std::vector<std::shared_ptr<Object>>& objects);
	static std::shared_ptr<Mesh> processMesh(aiMesh* assimpMesh, const aiScene* assimpScene, const std::string& directory, const glm::mat4 transform, Material& material);

	static int ModelLoader::loadTexture(const std::string& path, Texture::Type type, Material& material);
	
	static glm::mat4 aiMatrixToGLM(const aiMatrix4x4& from);
};