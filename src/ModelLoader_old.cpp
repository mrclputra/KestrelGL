#include "ModelLoader_old.h"
#include <stb_image.h>

std::shared_ptr<Object> ModelLoader_old::load(const std::string& path, std::string name, std::shared_ptr<Shader> shader) {
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
	object->name = name;

	//object->name = scene->mRootNode->mName.C_Str(); // get name from root node

	// traverse scene graph and populate object component 
	// start with identity matrix for root transform
	processNode(scene->mRootNode, scene, *object, directory, glm::mat4(1.0f));

	// set shader
	if (shader) {
		// if a custom shader is provided
		object->material->shader = shader;
	}
	else {
		// use default shader
		//object->shader = std::make_shared<Shader>(SHADER_DIR "model.vert", SHADER_DIR "model.frag");
		object->material->shader = std::make_shared<Shader>(SHADER_DIR "model.vert", SHADER_DIR "model_phong.frag");
	}

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

void ModelLoader_old::processNode(aiNode* node, const aiScene* scene, Object& object, const std::string& directory, const glm::mat4& parentTransform) {
	// get this node's transform and combine with parent
	glm::mat4 nodeTransform = aiMatrixToGlm(node->mTransformation);
	glm::mat4 accumulatedTransform = parentTransform * nodeTransform;

	// process current node's meshes with accumulated transform
	for (unsigned int i = 0; i < node->mNumMeshes; i++) {
		aiMesh* assimpMesh = scene->mMeshes[node->mMeshes[i]];
		auto mesh = processMesh(assimpMesh, scene, object, directory, accumulatedTransform);
		if (mesh) {
			object.meshes.push_back(mesh);
		}
	}

	// process children nodes, passing down accumulated transform
	for (unsigned int i = 0; i < node->mNumChildren; i++) {
		processNode(node->mChildren[i], scene, object, directory, accumulatedTransform);
	}
}

std::shared_ptr<Mesh> ModelLoader_old::processMesh(aiMesh* mesh, const aiScene* scene, Object& object, const std::string& directory, const glm::mat4& transform) {
	std::vector<Mesh::Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<int> texIndices;

	// for proper normal transformation
	// in the event the original mesh is transformed
	glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(transform)));

	// extract vertex data
	for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
		Mesh::Vertex vertex;

		// "foreach vertex, get the following data:"
		// position
		vertex.pos = transform * glm::vec4(
			mesh->mVertices[i].x,
			mesh->mVertices[i].y,
			mesh->mVertices[i].z,
			1.0f
		);

		// normals
		if (mesh->HasNormals()) {
			vertex.normal = normalMatrix * glm::vec3(
				mesh->mNormals[i].x,
				mesh->mNormals[i].y,
				mesh->mNormals[i].z
			);
			vertex.normal = glm::normalize(vertex.normal);
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
			vertex.tangent = glm::normalize(vertex.tangent);

			// bitangent
			vertex.bitangent = glm::vec3(
				mesh->mBitangents[i].x,
				mesh->mBitangents[i].y,
				mesh->mBitangents[i].z
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
		if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
			for (unsigned int i = 0; i < material->GetTextureCount(aiTextureType_DIFFUSE); i++) {
				aiString str;
				material->GetTexture(aiTextureType_DIFFUSE, i, &str);
				std::string texPath = directory + "/" + std::string(str.C_Str());
				int idx = loadTexture(texPath, Texture::Type::ALBEDO, object);
				texIndices.push_back(idx);
			}
		}

		// load normal maps
		//if (material->GetTextureCount(aiTextureType_NORMALS) > 0) {
		//	for (unsigned int i = 0; i < material->GetTextureCount(aiTextureType_NORMALS); i++) {
		//		aiString str;
		//		material->GetTexture(aiTextureType_NORMALS, i, &str);
		//		std::string texPath = directory + "/" + std::string(str.C_Str());
		//		int idx = loadTexture(texPath, Texture::Type::NORMAL, object);
		//		texIndices.push_back(idx);
		//	}
		//	object.material->useNormalMap = true;
		//}

		//// load metallic roughness maps
		//if (material->GetTextureCount(aiTextureType_GLTF_METALLIC_ROUGHNESS) > 0) {
		//	for (unsigned int i = 0; i < material->GetTextureCount(aiTextureType_GLTF_METALLIC_ROUGHNESS); i++) {
		//		aiString str;
		//		material->GetTexture(aiTextureType_GLTF_METALLIC_ROUGHNESS, i, &str);
		//		std::string texPath = directory + "/" + std::string(str.C_Str());
		//		int idx = loadTexture(texPath, Texture::Type::METALLIC_ROUGHNESS, object);
		//		texIndices.push_back(idx);
		//	}
		//}
	}

	// return processed mesh
	auto meshPtr = std::make_shared<Mesh>(vertices, indices);
	meshPtr->textureIndices = texIndices;
	return meshPtr;
}

int ModelLoader_old::loadTexture(const std::string& path, Texture::Type type, Object& object) {
	// TODO: check if texture is already loaded

	// load new texture
	//auto texture = std::make_shared<Texture>(path, type);
	auto texture = std::make_shared<Texture>(type);

	glGenTextures(1, &texture->id);

	int width, height, nrChannels;
	unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);

	if (data) {
		GLenum format;
		if (nrChannels == 1)
			format = GL_RED;
		else if (nrChannels == 2)
			format = GL_RG;
		else if (nrChannels == 3)
			format = GL_RGB;
		else if (nrChannels == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, texture->id);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		// configure texture parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else {
		logger.error("Failed to load texture: " + path);
		stbi_image_free(data);
	}

	// add to the object material
	object.material->textures.push_back(texture);

	logger.info("retrieved " + path);
	return static_cast<int>(object.material->textures.size() - 1);
}

glm::mat4 ModelLoader_old::aiMatrixToGlm(const aiMatrix4x4& from) {
	// Assimp uses row-major, GLM uses column-major
	return glm::mat4(
		from.a1, from.b1, from.c1, from.d1,
		from.a2, from.b2, from.c2, from.d2,
		from.a3, from.b3, from.c3, from.d3,
		from.a4, from.b4, from.c4, from.d4
	);
}