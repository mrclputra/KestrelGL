#include "ModelLoader.h"
#include <stb_image.h>

// todo: allow for custom shaders
std::vector<std::shared_ptr<Object>> ModelLoader::load(const std::string& path) {
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
		return {};
	}

	// get directory from file path
	std::string directory;
	size_t lastSlash = path.find_last_of("/\\");
	directory = (lastSlash != std::string::npos) ? path.substr(0, lastSlash) : ".";

	// objects vector
	auto objects = std::vector<std::shared_ptr<Object>>();

	// recurse and process all nodes
	processNode(assimpScene->mRootNode, assimpScene, directory, glm::mat4(1.0), objects);

	return objects;
}

void ModelLoader::processNode(aiNode* assimpNode, const aiScene *assimpScene, const std::string& directory, const glm::mat4& parentTransform, std::vector<std::shared_ptr<Object>>& objects) {
	// get local transformation
	// as it compounds on it's parent
	glm::mat4 local = aiMatrixToGLM(assimpNode->mTransformation);
	glm::mat4 global = parentTransform * local;

	auto object = std::make_shared<Object>("Object_" + std::string(assimpNode->mName.C_Str())	);
	object->material->shader = std::make_shared<Shader>(SHADER_DIR "model.vert", SHADER_DIR "model_phong.frag");

	// process current node's meshes
	for (unsigned int i = 0; i < assimpNode->mNumMeshes; i++) {
		aiMesh* assimpMesh = assimpScene->mMeshes[assimpNode->mMeshes[i]];
		
		auto mesh = processMesh(assimpMesh, assimpScene, directory, global, *object->material);
		if (mesh) {
			object->meshes.push_back(mesh);
		}
	}

	objects.push_back(object);

	// recurse into children
	for (unsigned int i = 0; i < assimpNode->mNumChildren; i++) {
		processNode(assimpNode->mChildren[i], assimpScene, directory, global, objects);
	}
}

std::shared_ptr<Mesh> ModelLoader::processMesh(aiMesh* assimpMesh, const aiScene* assimpScene, const std::string& directory, const glm::mat4 transform, Material& material) {
	std::vector<Mesh::Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<int> texIndices;
	// TODO: texture index?

	// for proper normal transformation
	// in the event the original mesh is transformed
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

	// TODO: process materials and textures
	// TODO: this should probably be done before loading vertices?

	//if (assimpMesh->mMaterialIndex >= 0) {
	//	// need to map this into my engine
	//	aiMaterial* assimpMaterial = assimpScene->mMaterials[assimpMesh->mMaterialIndex];

	//	// load albedo textures
	//	if (assimpMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
	//		for (unsigned int i = 0; i < assimpMaterial->GetTextureCount(aiTextureType_DIFFUSE); i++) {
	//			aiString str;
	//			assimpMaterial->GetTexture(aiTextureType_DIFFUSE, i, &str);
	//			std::string texPath = directory + "/" + std::string(str.C_Str());
	//			int idx = loadTexture(texPath, Texture::Type::ALBEDO, material);
	//			texIndices.push_back(idx);
	//		}
	//	}

	//	// todo: add more texture types later
	//}

	// return mesh
	auto mesh = std::make_shared<Mesh>(vertices, indices);
	//mesh->textureIndices = texIndices;
	return mesh;
}

// loads a texture
// returns a handler to the texture
int ModelLoader::loadTexture(const std::string& path, Texture::Type type, Material& material) {
	// check if texture is already loaded, need to think of a fast way to do this
	for (size_t i = 0; i < material.textures.size(); ++i) {
		if (material.textures[i]->path == path) return static_cast<int>(i);
	}
	
	auto texture = std::make_shared<Texture>(path, type);

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

		// configure texture parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		logger.info("Loaded texture: " + path + "\n\tat " + std::to_string(texture->id));
		stbi_image_free(data);
	}
	else {
		logger.error("Failed to load texture: " + path);
		stbi_image_free(data);
	}

	// add to the material
	material.textures.push_back(texture);
	return static_cast<int>(material.textures.size() - 1);
}

glm::mat4 ModelLoader::aiMatrixToGLM(const aiMatrix4x4& from) {
	// Assimp uses row-major, GLM uses column-major
	return glm::mat4(
		from.a1, from.b1, from.c1, from.d1,
		from.a2, from.b2, from.c2, from.d2,
		from.a3, from.b3, from.c3, from.d3,
		from.a4, from.b4, from.c4, from.d4
	);
}