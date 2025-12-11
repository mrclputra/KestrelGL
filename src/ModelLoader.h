#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <string>
#include <memory>
#include <vector>
#include <numeric>

#include <logger.h>

#include "Entity.h"

/// <summary>
/// <para>This class is responsible for constructing Entity instances from files</para>
/// <para>Makes use of ASSIMP to do this.</para>
/// </summary>
class ModelLoader {
public:
	/// <summary>
	/// Load a model file and return an Entity object with all of it's components
	/// </summary>
	/// <param name="path">The path to the model file</param>
	/// <returns></returns>
	static std::shared_ptr<Entity> load(const std::string& path);

private:
	/// <summary>
	/// <para>Traverses the Assimp scene graph and processes all nodes</para>
	/// </summary>
	/// <param name="node">The starting node</param>
	/// <param name="scene">The Assimp Scene instance</param>
	/// <param name="entity">The Entity which to save data into</param>
	static void processNode(aiNode* node, const aiScene* scene, Entity& entity);

	/// <summary>
	/// <para>Convert the Assimp mesh into our mesh format</para>
	/// </summary>
	/// <param name="mesh">The Assimp mesh to parse.</param>
	/// <param name="scene">The Assimp Scene instance</param>
	/// <returns>Mesh object</returns>
	static std::shared_ptr<Mesh> processMesh(aiMesh* mesh, const aiScene* scene);

	// TODO: in the future, figure out where use of materials falls into all of this
	//  maybe a new function called processMaterials or something, 
	//	but dont meshes need to store references to said materials? maybe somehow manage this issue at the Entity level?
	//static std::shared_ptr<Material> processMaterial(aiMaterial* material, const aiScene* scene, const std::string& directory);
};