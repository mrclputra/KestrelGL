#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <string>
#include <memory>
#include <vector>
#include <numeric>

#include <logger.h>

#include "Object.h"

/// <summary>
/// <para>This class is responsible for constructing Object instances from files</para>
/// <para>Makes use of ASSIMP to do this.</para>
/// </summary>
class ModelLoader {
public:
	/// <summary>
	/// Loads a 3d model file.
	/// </summary>
	/// <param name="path">The path to the model file</param>
	/// <returns>An Object object</returns>
	static std::shared_ptr<Object> load(const std::string& path, std::shared_ptr<Shader> shader = nullptr, std::string = "NONAME");

private:
	/// <summary>
	/// <para>Traverses the Assimp scene graph and processes all nodes</para>
	/// </summary>
	/// <param name="node">The starting node</param>
	/// <param name="scene">The Assimp Scene instance</param>
	/// <param name="entity">The Object which to save data into</param>
	/// <param name="directory">Directory where the model file is located</param>
	/// <param name="transform">Accumulated transformation from parent nodes</param>
	static void processNode(aiNode* node, const aiScene* scene, Object& entity, const std::string& directory, const glm::mat4& transform);

	/// <summary>
	/// <para>Convert the Assimp mesh into our mesh format</para>
	/// </summary>
	/// <param name="mesh">The Assimp mesh to parse.</param>
	/// <param name="scene">The Assimp Scene instance</param>
	/// <param name="directory">Directory where the model file is located</param>
	/// <param name="transform">Transformation to bake into vertices</param>
	/// <returns>Mesh object</returns>
	static std::shared_ptr<Mesh> processMesh(aiMesh* mesh, const aiScene* scene, Object& object, const std::string& directory, const glm::mat4& transform);

	// TODO: in the future, figure out where use of materials falls into all of this
	//  maybe a new function called processMaterials or something, 
	//	but dont meshes need to store references to said materials? maybe somehow manage this issue at the Object level?
	//static std::shared_ptr<Material> processMaterial(aiMaterial* material, const aiScene* scene, const std::string& directory);

	/// <summary>
	/// <para>Loads a texture and adds it to the object if not already loaded</para>
	/// </summary>
	/// <param name="path"></param>
	/// <param name="type"></param>
	/// <param name="object"></param>
	/// <returns></returns>
	static int loadTexture(const std::string& path, Texture::Type type, Object& object);

	/// <summary>
	/// <para>Converts Assimp matrix to GLM matrix</para>
	/// </summary>
	static glm::mat4 aiMatrixToGlm(const aiMatrix4x4& from);
};