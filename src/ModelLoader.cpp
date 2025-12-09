#include "ModelLoader.h"

std::shared_ptr<Entity> ModelLoader::load(const std::string& path, std::shared_ptr<Shader> shader) {
    //logger.info("Loading model: " + path);

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, 
		aiProcess_Triangulate |
		aiProcess_GenSmoothNormals |
		aiProcess_FlipUVs |
		aiProcess_CalcTangentSpace);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		logger.error("Failed to load model: " + std::string(importer.GetErrorString()));
		return nullptr;
	}

	// get directory from file path
	std::string directory;
	size_t lastSlash = path.find_last_of("/\\");
	directory = (lastSlash != std::string::npos) ? path.substr(0, lastSlash) : ".";

	// create entity object
	auto entity = std::make_shared<Entity>(path, shader);
    entity->name = scene->mRootNode->mName.C_Str();

    // populate entity meshes vector
	processNode(scene->mRootNode, scene, *entity, directory);

    logger.info("Entity created: " + entity->name +
        ", shader: 0x" + std::to_string(reinterpret_cast<uintptr_t>(shader.get())) +
        ", meshes: 0x" + std::to_string(reinterpret_cast<uintptr_t>(entity->meshes.back().get())));
	return entity;
}

void ModelLoader::processNode(aiNode* node, const aiScene* scene, Entity& entity, const std::string& directory) {
	// process all meshes in current node
	for (unsigned int i = 0; i < node->mNumMeshes; i++) {
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		auto processedMesh = processMesh(mesh, scene, directory);
		if (processedMesh) {
			entity.meshes.push_back(processedMesh);
		}
	}

	// recursively process children
	for (unsigned int i = 0; i < node->mNumChildren; i++) {
		processNode(node->mChildren[i], scene, entity, directory);
	}
}

std::shared_ptr<Mesh> ModelLoader::processMesh(aiMesh* mesh, const aiScene* scene, const std::string& directory) {
    std::vector<Mesh::Vertex> vertices;
    std::vector<unsigned int> indices;

    // extract vertex data
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Mesh::Vertex vertex;

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
                1.0f - mesh->mTextureCoords[0][i].y
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

    // TODO: load textures when Entity supports them
    // aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

    auto newMesh = std::make_shared<Mesh>(vertices, indices);
    newMesh->upload();
    return newMesh;
}

unsigned int ModelLoader::loadTexture(const std::string& path, const std::string& directory) {
    std::string filename = path;

    // check if path is absolute
    bool isAbsolute = (filename[0] == '/' || filename[0] == '\\' ||
        (filename.length() > 1 && filename[1] == ':'));

    if (!isAbsolute) {
        filename = directory + "/" + filename;
    }

    logger.info("Loading texture: " + filename);

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, channels;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &channels, 0);

    if (data) {
        GLenum format = GL_RGB;
        if (channels == 1) format = GL_RED;
        else if (channels == 2) format = GL_RG;
        else if (channels == 3) format = GL_RGB;
        else if (channels == 4) format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else {
        std::cerr << "Failed to load texture: " << filename << "\n";
        stbi_image_free(data);
    }

    return textureID;
}