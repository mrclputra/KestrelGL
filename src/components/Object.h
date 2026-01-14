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
public:
	Object(const std::string& name = "Object") {
		this->name = name;

		//shader = std::make_shared<Shader>();
		material = std::make_shared<Material>(); // add a material by default?
	}
	~Object() = default;

	// per-frame logic
	void update(float deltaTime) {}

	// the components below must be assigned and valid before any draws are made
	// as such, the modelloader class is responsible for doing this

	// object components
	Transform transform;
	//std::shared_ptr<Shader> shader; // todo: move this under material
	std::shared_ptr<Material> material;
	std::vector<std::shared_ptr<Mesh>> meshes;

	std::string name;
};