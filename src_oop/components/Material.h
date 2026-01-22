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
	// these are used in the case that a texture of the same type is not found, or if we specifically tell the renderer to
	glm::vec4 albedo = glm::vec4(0.98f, 0.98f, 0.98f, 1.0f);
	float metalness = 0.0f;
	float roughness = 0.2f;
	// these are to be expanded; see https://www.khronos.org/gltf/pbr
	//float emission;

	// TODO: toggle between texture and parameters

	// components
	std::shared_ptr<Shader> shader;
	std::vector<std::shared_ptr<Texture>> textures;
};