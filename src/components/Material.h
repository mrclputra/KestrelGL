#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <string>

#include "Shader.h"
#include "Texture.h"

class Material {
public:
	std::shared_ptr<Shader> shader;

	// parameters
	// these are to be expanded based on the PBR model I use in the future
	glm::vec4 baseColor = glm::vec4(1.0f);
	float metallic = 1.0f;
	float roughness = 1.0f;
	float normalScale = 1.0f;
	float emissiveStrength = 0.0f;

	// textures
	// these are optional
	std::shared_ptr<Texture> albedoMap;
	std::shared_ptr<Texture> normalMap;
	std::shared_ptr<Texture> metallicRoughnessMap;
	std::shared_ptr<Texture> occlusionMap;
	std::shared_ptr<Texture> emissiveMap;

	void bind() const {
		shader->use();

		shader->setVec4("u_BaseColor", baseColor);
		shader->setFloat("u_Metallic", metallic);
		shader->setFloat("u_Roughness", roughness);
		shader->setFloat("u_NormalScale", normalScale);
		shader->setFloat("u_EmissiveStrength", emissiveStrength);

		int slot = 0;

		auto bindTex = [&](const char* flag, const char* sampler, const std::shared_ptr<Texture>& tex) {
			bool has = (bool)tex;
			shader->setBool(flag, has);
			if (has) {
				tex->bind(slot);
				shader->setInt(sampler, slot);
				slot++;
			}
		};

		bindTex("u_HasAlbedoMap", "u_AlbedoMap", albedoMap);
		bindTex("u_HasNormalMap", "u_NormalMap", normalMap);
		bindTex("u_HasMetallicRoughnessMap", "u_MetallicRoughnessMap", metallicRoughnessMap);
		bindTex("u_HasOcclusionMap", "u_OcclusionMap", occlusionMap);
		bindTex("u_HasEmissiveMap", "u_EmissiveMap", emissiveMap);
	}
};