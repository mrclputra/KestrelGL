#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <string>

#include "Mesh.h"
#include "Material.h"
#include "Transform.h"

#include <logger.h>

class Object {
public:
	// constructor
	Object(const std::string& name = "noname") {
		material = std::make_shared<Material>();
		this->name = name;
	}
	~Object() = default;

	// per-frame logic
	void update(float deltaTime) {}

	// the variables below must be assigned and valid before any draws are made
	// as such, the modelloader class is responsible for doing this

	// components
	std::string name;
	Transform transform;

	std::vector<std::shared_ptr<Mesh>> meshes;
	std::shared_ptr<Material> material;

	// TODO: figure out how to do parent child references?
	//	though I don't think it is necessary
};