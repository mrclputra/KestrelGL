#include "ModelLoader.h"
#include "stb_image.h"

namespace ModelLoader {

	// forward declarations
	static void processNodeAsObject(
		aiNode* assimpNode,
		const aiScene* assimpScene,
		const std::string& directory,
		std::vector<std::shared_ptr<Object>>& objects,
		std::vector<std::shared_ptr<Texture>>& textureCache
	);
	static std::shared_ptr<Mesh> processMesh(
		aiMesh* assimpMesh,
		const aiScene* assimpScene,
		const std::string& directory,
		Object& object,
		std::vector<std::shared_ptr<Texture>>& textureCache
	);
	static int loadTexture(
		const std::string& path,
		Texture::Type type,
		Object& object,
		std::vector<std::shared_ptr<Texture>>& textureCache
	);
	static std::string getDirectory(const std::string& path);
	static glm::mat4 aiMatrixToGLM(const aiMatrix4x4& from);

	// load a file as a vector of objects
	std::vector<std::shared_ptr<Object>> loadAsObjects(
		const std::string& path, 
		std::vector<std::shared_ptr<Texture>>& textureCache, 
		glm::vec3 scale
	) {
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

		std::string directory = getDirectory(path);

		// each aiNode is it's own object
		// as node's can exist further down the assimp tree, we flatten it into a single vector that get's passed through recursion
		std::vector<std::shared_ptr<Object>> objects;
		processNodeAsObject(assimpScene->mRootNode, assimpScene, directory, objects, textureCache);

		// it's very important that we use the same shader instance across all the objects
		// todo: this should be set externally from outside this function in the future
		auto shader = std::make_shared<Shader>(SHADER_DIR "model.vert", SHADER_DIR "model_phong.frag");
		for (auto& object : objects) {
			object->material->shader = shader;
			object->transform.scale = scale;
		}

		return objects;
	}

	static void processNodeAsObject(
		aiNode* assimpNode,
		const aiScene* assimpScene,
		const std::string& directory,
		std::vector<std::shared_ptr<Object>>& objects,
		std::vector<std::shared_ptr<Texture>>& textureCache
	) {
		logger.info("processing " + std::string(assimpNode->mName.C_Str()));

		if (assimpNode->mNumMeshes > 0) {
			auto object = std::make_shared<Object>();
			object->name = std::string(assimpNode->mName.C_Str());

			for (unsigned int i = 0; i < assimpNode->mNumMeshes; i++) {
				aiMesh* assimpMesh = assimpScene->mMeshes[assimpNode->mMeshes[i]];
				auto mesh = processMesh(assimpMesh, assimpScene, directory, *object, textureCache);
				if (mesh) object->meshes.push_back(std::move(mesh));
			}

			objects.push_back(std::move(object));
		}

		// recurse into children
		for (unsigned int i = 0; i < assimpNode->mNumChildren; i++) {
			processNodeAsObject(assimpNode->mChildren[i], assimpScene, directory, objects, textureCache);
		}
	}

	static std::shared_ptr<Mesh> processMesh(
		aiMesh* assimpMesh,
		const aiScene* assimpScene,
		const std::string& directory,
		Object& object,
		std::vector<std::shared_ptr<Texture>>& textureCache
	) {
		std::vector<Mesh::Vertex> vertices;
		std::vector<unsigned int> indices;
		std::vector<int> texIndices;

		// vertices
		for (unsigned int i = 0; i < assimpMesh->mNumVertices; i++) {
			Mesh::Vertex vertex;

			// position
			vertex.pos = glm::vec3(
				assimpMesh->mVertices[i].x,
				assimpMesh->mVertices[i].y,
				assimpMesh->mVertices[i].z
			);

			// normals
			if (assimpMesh->HasNormals()) {
				vertex.normal = glm::vec3(
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

		// indices
		for (unsigned int i = 0; i < assimpMesh->mNumFaces; i++) {
			aiFace assimpFace = assimpMesh->mFaces[i];
			for (unsigned int j = 0; j < assimpFace.mNumIndices; j++) {
				indices.push_back(assimpFace.mIndices[j]);
			}
		}

		// textures/materials
		if (assimpMesh->mMaterialIndex >= 0) {
			aiMaterial* assimpMaterial = assimpScene->mMaterials[assimpMesh->mMaterialIndex];

			float opacity = 1.0f;
			if (AI_SUCCESS == assimpMaterial->Get(AI_MATKEY_OPACITY, opacity)) {
				if (opacity < 1.0f) object.material->isTransparent = true;
			}

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

	static int loadTexture(
		const std::string& path,
		Texture::Type type,
		Object& object,
		std::vector<std::shared_ptr<Texture>>& textureCache
	) {
		for (int i = 0; i < textureCache.size(); i++) {
			if (textureCache[i]->path == path)
				return i; // already loaded
		}

		auto texture = std::make_shared<Texture>(type, path);
		glGenTextures(1, &texture->id);

		int width, height, nrChannels;
		unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);

		if (data) {
			GLenum internalFormat = GL_RGBA8;
			GLenum dataFormat = GL_RGBA;

			if (nrChannels == 1) { internalFormat = GL_R8; dataFormat = GL_RED; }
			if (nrChannels == 2) { internalFormat = GL_RG8; dataFormat = GL_RG; }
			if (nrChannels == 3) { internalFormat = GL_RGB8; dataFormat = GL_RGB; }
			if (nrChannels == 4) { internalFormat = GL_RGBA8; dataFormat = GL_RGBA; }

			glBindTexture(GL_TEXTURE_2D, texture->id);
			glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			logger.info("Loaded texture: " + path);
			stbi_image_free(data);
		}
		else {
			logger.error("Failed to load texture: " + path);
			stbi_image_free(data);
		}

		textureCache.push_back(texture);
		return static_cast<int>(textureCache.size() - 1); // new index
	}

	// utility functions
	static glm::mat4 aiMatrixToGLM(const aiMatrix4x4& from) {
		return glm::mat4(
			from.a1, from.b1, from.c1, from.d1,
			from.a2, from.b2, from.c2, from.d2,
			from.a3, from.b3, from.c3, from.d3,
			from.a4, from.b4, from.c4, from.d4
		);
	}

	static std::string getDirectory(const std::string& path) {
		unsigned int lastSlash = path.find_last_of("/\\");
		return (lastSlash != std::string::npos) ? path.substr(0, lastSlash) : ".";
	}
}