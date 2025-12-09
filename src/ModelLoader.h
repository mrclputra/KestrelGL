#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <stb_image.h>

#include <logger.h>

#include <string>
#include <vector>
#include <memory>

#include "Entity.h"
#include "Mesh.h"

class ModelLoader {
public:
	// load a model file and return an Entity with all it's meshes
	static std::shared_ptr<Entity> load(const std::string& path, std::shared_ptr<Shader> shader);

private:
	// recursively process nodes
	static void processNode(aiNode* node, const aiScene* scene, Entity& entity, const std::string& directory);

	// convert assimp mesh to our mesh format
	static std::shared_ptr<Mesh> processMesh(aiMesh* mesh, const aiScene* scene, const std::string& directory);

	// load a texture from file
	static unsigned int loadTexture(const std::string& path, const std::string& directory);
};