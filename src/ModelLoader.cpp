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

	// create empty entity object
	auto entity = std::make_shared<Object>(path);
	entity->name = scene->mRootNode->mName.C_Str(); // get name from root node

	// traverse scene graph and populate entity component 
	processNode(scene->mRootNode, scene, *entity);

	logger.info(
		"Object created: " + entity->name + " "
		"meshes: " + "[" + std::accumulate(
			entity->meshes.begin(), entity->meshes.end(), std::string{},
			[](const std::string& a, const std::shared_ptr<Mesh>& b) {
				return a.empty() ? ("0x" + std::to_string(reinterpret_cast<uintptr_t>(b.get())))
					: a + ", 0x" + std::to_string(reinterpret_cast<uintptr_t>(b.get()));
			}
		) + "]"
	);
	return entity;
}

void ModelLoader::processNode(aiNode* node, const aiScene* scene, Object& entity) {
	// process current node
	for (unsigned int i = 0; i < node->mNumMeshes; i++) {
		aiMesh* assimpMesh = scene->mMeshes[node->mMeshes[i]];
		auto mesh = processMesh(assimpMesh, scene);
		if (mesh) {
			entity.meshes.push_back(mesh);
		}

		// TODO: figure out how materials are stored in ASSIMP
		//	and then pass it into the entity
	}

	// process children nodes,
	for (unsigned int i = 0; i < node->mNumChildren; i++) {
		processNode(node->mChildren[i], scene, entity);
	}
}

std::shared_ptr<Mesh> ModelLoader::processMesh(aiMesh* mesh, const aiScene* scene) {
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
				1.0 - mesh->mTextureCoords[0][i].y // flip
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

	// return processed mesh
	return std::make_shared<Mesh>(vertices, indices);
}