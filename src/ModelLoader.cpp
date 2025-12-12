#include "ModelLoader.h"

std::shared_ptr<Object> ModelLoader::load(const std::string& path) {
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path,
		aiProcess_Triangulate |
		aiProcess_GenSmoothNormals |
		aiProcess_FlipUVs |
		aiProcess_CalcTangentSpace
	);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		logger.error("failed to load model: " + std::string(importer.GetErrorString()));
		return nullptr;
	}

	// get directory from file path
	std::string directory;
	size_t lastSlash = path.find_last_of("/\\");
	directory = (lastSlash != std::string::npos) ? path.substr(0, lastSlash) : ".";

	// create empty object
	auto object = std::make_shared<Object>(path);
	object->name = scene->mRootNode->mName.C_Str(); // get name from root node

	// traverse scene graph and populate object component 
	processNode(scene->mRootNode, scene, *object, directory);

	logger.info(
		"Object created: " + object->name + " "
		"meshes: " + "[" + std::accumulate(
			object->meshes.begin(), object->meshes.end(), std::string{},
			[](const std::string& a, const std::shared_ptr<Mesh>& b) {
				return a.empty() ? ("0x" + std::to_string(reinterpret_cast<uintptr_t>(b.get())))
					: a + ", 0x" + std::to_string(reinterpret_cast<uintptr_t>(b.get()));
			}
		) + "]"
	);
	return object;
}

void ModelLoader::processNode(aiNode* node, const aiScene* scene, Object& object, const std::string& directory) {
	// process current node
	for (unsigned int i = 0; i < node->mNumMeshes; i++) {
		aiMesh* assimpMesh = scene->mMeshes[node->mMeshes[i]];
		auto mesh = processMesh(assimpMesh, scene, object, directory);
		if (mesh) {
			object.meshes.push_back(mesh);
		}
	}

	// process children nodes,
	for (unsigned int i = 0; i < node->mNumChildren; i++) {
		processNode(node->mChildren[i], scene, object, directory);
	}
}

std::shared_ptr<Mesh> ModelLoader::processMesh(aiMesh* mesh, const aiScene* scene, Object& object, const std::string& directory) {
	std::vector<Mesh::Vertex> vertices;
	std::vector<unsigned int> indices;

	// extract vertex data
	for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
		Mesh::Vertex vertex;

		// "foreach vertex, get the following data:"
		// position
		vertex.pos = glm::vec3(
			mesh->mVertices[i].x,
			mesh->mVertices[i].y,
			mesh->mVertices[i].z
		);

		// normals
		if (mesh->HasNormals()) {
			vertex.normal = glm::vec3(
				mesh->mNormals[i].x,
				mesh->mNormals[i].y,
				mesh->mNormals[i].z
			);
		}
		else {
			vertex.normal = glm::vec3(0.0f);
		}

		// texture coordinates
		if (mesh->mTextureCoords[0]) {
			vertex.uv = glm::vec2(
				mesh->mTextureCoords[0][i].x,
				mesh->mTextureCoords[0][i].y
			);

			// tangent
			vertex.tangent = glm::vec3(
				mesh->mTangents[i].x,
				mesh->mTangents[i].y,
				mesh->mTangents[i].z
			);

			// bitangent
			vertex.bitangent = glm::vec3(
				mesh->mBitangents[i].x,
				mesh->mBitangents[i].y,
				mesh->mBitangents[i].z
			);
		}
		else {
			vertex.uv = glm::vec2(0.0f);
			vertex.tangent = glm::vec3(0.0f);
			vertex.bitangent = glm::vec3(0.0f);
		}

		vertices.push_back(vertex);
	}

	// extract indices from faces
	for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++) {
			indices.push_back(face.mIndices[j]);
		}
	}

	// process materials and textures
	if (mesh->mMaterialIndex >= 0) {
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

		// load albedo textures
		for (unsigned int i = 0; i < material->GetTextureCount(aiTextureType_DIFFUSE); i++) {
			aiString str;
			material->GetTexture(aiTextureType_DIFFUSE, i, &str);
			std::string texPath = directory + "/" + std::string(str.C_Str());
			loadTexture(texPath, Texture::Type::ALBEDO, object);
		}

		// load normal maps
		for (unsigned int i = 0; i < material->GetTextureCount(aiTextureType_NORMALS); i++) {
			aiString str;
			material->GetTexture(aiTextureType_NORMALS, i, &str);
			std::string texPath = directory + "/" + std::string(str.C_Str());
			loadTexture(texPath, Texture::Type::NORMAL, object);
		}

		// load metallic roughness maps
		for (unsigned int i = 0; i < material->GetTextureCount(aiTextureType_GLTF_METALLIC_ROUGHNESS); i++) {
			aiString str;
			material->GetTexture(aiTextureType_GLTF_METALLIC_ROUGHNESS, i, &str);
			std::string texPath = directory + "/" + std::string(str.C_Str());
			loadTexture(texPath, Texture::Type::METALLIC_ROUGHNESS, object);
		}

	}

	// return processed mesh
	return std::make_shared<Mesh>(vertices, indices);
}

int ModelLoader::loadTexture(const std::string& path, Texture::Type type, Object& object) {
	// check if texture is already loaded
	for (size_t i = 0; i < object.textures.size(); i++) {
		if (object.textures[i]->path == path) {
			// texture already exists
			return static_cast<int>(i); 
		}
	}

	// load new texture
	auto texture = std::make_shared<Texture>(path, type);
	object.textures.push_back(texture);
	return static_cast<int>(object.textures.size() - 1);
}