#include "ModelLoader.h"
#include <stb_image.h>

// I think it is fine if we make the loaded file as a single object
//	for one, it is great for performance as we do not have to rebind N-many object transformations every frame
//	second, helps for organization, as I this engine is meant for rendering only

// if we want to make changes to the model, that should be done externally instead of here

std::shared_ptr<Object> ModelLoader::load(const std::string& path, std::vector<std::shared_ptr<Texture>>& textureCache, const std::string& name) {
	Assimp::Importer importer;
	const aiScene* assimpScene = importer.ReadFile(path,
		// post-processing flags
		aiProcess_Triangulate |
		aiProcess_CalcTangentSpace |
		aiProcess_GenSmoothNormals |
		aiProcess_FlipUVs
	);

	if (!assimpScene || assimpScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !assimpScene->mRootNode) {
		logger.error("failed to load: " + std::string(importer.GetErrorString()));
		return nullptr;
	}

	// get directory from file path
	std::string directory;
	size_t lastSlash = path.find_last_of("/\\");
	directory = (lastSlash != std::string::npos) ? path.substr(0, lastSlash) : ".";

	// single object
	auto object = std::make_shared<Object>(name);
	processNode(assimpScene->mRootNode, assimpScene, directory, glm::mat4(1.0), *object, textureCache);
	object->material->shader = std::make_shared<Shader>(SHADER_DIR "model.vert", SHADER_DIR "model_phong.frag");
	return object;
}

std::vector<std::shared_ptr<Object>> ModelLoader::loadAsMultiple(const std::string& path, std::vector<std::shared_ptr<Texture>>& textureCache) {
	Assimp::Importer importer;
	const aiScene* assimpScene = importer.ReadFile(path,
		aiProcess_Triangulate |
		aiProcess_CalcTangentSpace |
		aiProcess_GenSmoothNormals |
		aiProcess_FlipUVs
	);

	if (!assimpScene || assimpScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !assimpScene->mRootNode) {
		logger.error("failed to load: " + std::string(importer.GetErrorString()));
		return {};
	}

	std::string directory;
	size_t lastSlash = path.find_last_of("/\\");
	directory = (lastSlash != std::string::npos) ? path.substr(0, lastSlash) : ".";

	// each aiNode is it's own object
	std::vector<std::shared_ptr<Object>> objects;
	processNodeAsObject(assimpScene->mRootNode, assimpScene, directory, glm::mat4(1.0), objects, textureCache);

	auto shader = std::make_shared<Shader>(SHADER_DIR "model.vert", SHADER_DIR "model_phong.frag");

	for (auto& obj : objects) {
		obj->material->shader = shader;
	}

	return objects;
}

void ModelLoader::processNode(aiNode* assimpNode, const aiScene* assimpScene, const std::string& directory, const glm::mat4 transform, Object& object, std::vector<std::shared_ptr<Texture>>& textureCache) {
	// get world space transformation
	glm::mat4 local = aiMatrixtoGLM(assimpNode->mTransformation);
	glm::mat4 global = transform * local;

	// process current node's meshes
	for (unsigned int i = 0; i < assimpNode->mNumMeshes; i++) {
		aiMesh* assimpMesh = assimpScene->mMeshes[assimpNode->mMeshes[i]];
		auto mesh = processMesh(assimpMesh, assimpScene, directory, global, object, textureCache);
		if (mesh) {
			object.meshes.push_back(mesh);
		}
	}

	// recurse and process all children nodes
	for (unsigned int i = 0; i < assimpNode->mNumChildren; i++) {
		processNode(assimpNode->mChildren[i], assimpScene, directory, global, object, textureCache);
	}
}

// TODO: make a function to place object origin at center of vertices, then apply that to my own transform
//	instead of having it baked in the vertices
void ModelLoader::processNodeAsObject(aiNode* assimpNode, const aiScene* assimpScene, const std::string& directory, const glm::mat4 transform, std::vector<std::shared_ptr<Object>>& objects, std::vector<std::shared_ptr<Texture>>& textureCache) {
	glm::mat4 local = aiMatrixtoGLM(assimpNode->mTransformation);
	glm::mat4 global = transform * local;

	// create a new object for this node if it has meshes
	if (assimpNode->mNumMeshes > 0) {
		auto object = std::make_shared<Object>();
		object->name = std::string(assimpNode->mName.C_Str());

		glm::vec3 scale;
		glm::quat rotation;
		glm::vec3 translation;
		glm::vec3 skew;
		glm::vec4 perspective;
		glm::decompose(global, scale, rotation, translation, skew, perspective);

		object->transform.position = translation;
		object->transform.rotation = glm::degrees(glm::eulerAngles(rotation));
		object->transform.scale = scale;

		for (unsigned int i = 0; i < assimpNode->mNumMeshes; i++) {
			aiMesh* assimpMesh = assimpScene->mMeshes[assimpNode->mMeshes[i]];
			auto mesh = processMesh(assimpMesh, assimpScene, directory, glm::mat4(1.0f), *object, textureCache);
			if (mesh) {
				object->meshes.push_back(mesh);
			}
		}

		objects.push_back(object);
	}

	// recurse into children
	for (unsigned int i = 0; i < assimpNode->mNumChildren; i++) {
		processNodeAsObject(assimpNode->mChildren[i], assimpScene, directory, global, objects, textureCache);
	}
}

std::shared_ptr<Mesh> ModelLoader::processMesh(aiMesh* assimpMesh, const aiScene* assimpScene, const std::string& directory, const glm::mat4 transform, Object& object, std::vector<std::shared_ptr<Texture>>& textureCache) {
	std::vector<Mesh::Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<int> texIndices;

	// for proper normal transformation
	glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(transform)));

	// extract vertex data
	for (unsigned int i = 0; i < assimpMesh->mNumVertices; i++) {
		Mesh::Vertex vertex;

		// "foreach vertex, get the following data:"
		// position
		vertex.pos = transform * glm::vec4(
			assimpMesh->mVertices[i].x,
			assimpMesh->mVertices[i].y,
			assimpMesh->mVertices[i].z,
			1.0f
		);

		// normals
		if (assimpMesh->HasNormals()) {
			vertex.normal = normalMatrix * glm::vec3(
				assimpMesh->mNormals[i].x,
				assimpMesh->mNormals[i].y,
				assimpMesh->mNormals[i].z
			);
			vertex.normal = glm::normalize(vertex.normal);
		}
		else {
			vertex.normal = glm::vec3(0.0f);
		}

		// texture coordinates
		if (assimpMesh->mTextureCoords[0]) {
			vertex.uv = glm::vec2(
				assimpMesh->mTextureCoords[0][i].x,
				assimpMesh->mTextureCoords[0][i].y
			);

			// tangent
			vertex.tangent = glm::vec3(
				assimpMesh->mTangents[i].x,
				assimpMesh->mTangents[i].y,
				assimpMesh->mTangents[i].z
			);
			vertex.tangent = glm::normalize(vertex.tangent);

			// bitangent
			vertex.bitangent = glm::vec3(
				assimpMesh->mBitangents[i].x,
				assimpMesh->mBitangents[i].y,
				assimpMesh->mBitangents[i].z
			);
			vertex.bitangent = glm::normalize(vertex.bitangent);
		}
		else {
			vertex.uv = glm::vec2(0.0f);
			vertex.tangent = glm::vec3(0.0f);
			vertex.bitangent = glm::vec3(0.0f);
		}

		vertices.push_back(vertex);
	}

	// extract indices
	for (unsigned int i = 0; i < assimpMesh->mNumFaces; i++) {
		aiFace face = assimpMesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++) {
			indices.push_back(face.mIndices[j]);
		}
	}

	// extract textures
	if (assimpMesh->mMaterialIndex >= 0) {
		aiMaterial* assimpMaterial = assimpScene->mMaterials[assimpMesh->mMaterialIndex];

		float opacity = 1.0f;
		if (AI_SUCCESS == assimpMaterial->Get(AI_MATKEY_OPACITY, opacity)) {
			if (opacity < 1.0f) object.material->isTransparent = true;
		}

		// load albedo textures
		if (assimpMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
			for (unsigned int i = 0; i < assimpMaterial->GetTextureCount(aiTextureType_DIFFUSE); i++) {
				aiString str;
				assimpMaterial->GetTexture(aiTextureType_DIFFUSE, i, &str);
				std::string texPath = directory + "/" + std::string(str.C_Str());
				int idx = loadTexture(texPath, Texture::Type::ALBEDO, object, textureCache);
				texIndices.push_back(idx);
			}
		}
	}

	logger.info("loaded mesh: " + std::string(assimpMesh->mName.C_Str()));

	auto mesh = std::make_shared<Mesh>(vertices, indices);
	mesh->texIndices = texIndices;
	return mesh;
}

// load a texture
int ModelLoader::loadTexture(const std::string& path, Texture::Type type, Object& object, std::vector<std::shared_ptr<Texture>>& textureCache) {
	// check if texture is already loaded
	for (int i = 0; i < textureCache.size(); i++) {
		if (textureCache[i]->path == path) {
			return i;
		}
	}

	auto texture = std::make_shared<Texture>(type, path);
	glGenTextures(1, &texture->id);

	int width, height, nrChannels;
	unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);

	if (data) {
		GLenum format;
		if (nrChannels == 1)
			format = GL_RED;
		if (nrChannels == 2)
			format = GL_RG;
		if (nrChannels == 3)
			format = GL_RGB;
		if (nrChannels == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, texture->id);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		logger.info("Loaded texture: " + path);
		stbi_image_free(data);
	}
	else {
		logger.error("Failed to load texture: " + path);
		stbi_image_free(data);
	}

	textureCache.push_back(texture);
	return static_cast<int>(textureCache.size() - 1); // index
}

glm::mat4 ModelLoader::aiMatrixtoGLM(const aiMatrix4x4& from) {
	// Assimp uses row-major, GLM uses column-major
	return glm::mat4(
		from.a1, from.b1, from.c1, from.d1,
		from.a2, from.b2, from.c2, from.d2,
		from.a3, from.b3, from.c3, from.d3,
		from.a4, from.b4, from.c4, from.d4
	);
}