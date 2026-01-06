#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <string>

#include <logger.h>

#include "components/Mesh.h"
#include "components/Transform.h"
#include "components/Material.h"
#include "components/Shader.h"

class Object {
	// the main idea behind this class is that it acts as sort of a container for attributes;
	//	during the render pass, we iteratre through all components and set shader uniforms

public:
	Object(const std::string& name = "Object") : name(name) {
		shader = std::make_shared<Shader>();
		material = std::make_shared<Material>();
	}
	~Object() = default;

	// per-frame logic
	void update(float deltaTime) {}

	// the components below must be assigned and valid before any draws are made
	// as such, the modelloader class is responsible for doing this

	// object components
	Transform transform;
	std::shared_ptr<Shader> shader;
	std::shared_ptr<Material> material; // maybe this isnt initialized?
	std::vector<std::shared_ptr<Mesh>> meshes;

	// THIS HAS BEEN MOVED TO THE MATERIAL
	//std::vector<std::shared_ptr<Texture>> textures;
	
	// TODO: considering moving textures to a global vector at the scene level instead; 
	//	as multiple meshes may reference the same texture

	std::string name;

	// todo: as this shold be in the material,
	//	all references to metalness/roughness should go to the material class instead of this object class
	//float metalnessFac = 0.0f; // TEMPORARY
	//float roughnessFac = 0.32f; // TEMPORARY
};