#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <string>

#include "Texture.h"
#include "Shader.h"

class Material {
public:
	// constructors
	Material() = default;
	~Material() = default;

	// PBR parameters
	glm::vec4 albedo = glm::vec4(0.98, 0.98, 0.98, 1.0); // rgba
	float metalness = 0.0f; // 0-1
	float roughness = 0.2f; // 0-1
	// to be expanded... according to the gltf convention
	// https://www.khronos.org/gltf/pbr
	//float alpha;
	//float emission;

	//// toggles, these should be set through the modelloader class
	//bool useAlbedoMap = false;
	//bool useNormalMap = false;
	//bool useMetRoughMap = false;
	//// to be implemented:
	//bool useAOMap = false;
	//bool useEmissionMap = false;
	
	// associated shader
	std::shared_ptr<Shader> shader;

	// textures
	// TODO: considering moving textures to a global vector at the scene level instead; 
	//	as multiple meshes may reference the same texture
	std::vector<std::shared_ptr<Texture>> textures;

	// if a specific Texture::Type for albedo, metalness, or roughness exists,
	//	we should override the associated PBR parameter in the shader, also decided by the flag
	//	;we should upload it to the GPU regardless of the useflag state
};