#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <string>

#include "Texture.h"

class Material {
public:
	Material() = default;
	~Material() = default;

	// PBR parameters
	glm::vec3 albedo = glm::vec3(0.98); // rgb
	float metalness = 0.0f; // 0-1
	float roughness = 0.2f; // 0-1

	// toggles
	bool useAlbedoTexture = true;
	bool useMetRoughTexture = true;
	
	// to be expanded...
	// https://www.khronos.org/gltf/pbr
	//float alpha;
	//float emission;
	
	// textures
	std::vector<std::shared_ptr<Texture>> textures;

	// todo: if a specific Texture::Type for albedo, metalness, or roughness exists,
	//	we should override the associated PBR parameter in the shader
};