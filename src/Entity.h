#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <string>

#include <logger.h>
#include <shader.h>

#include "components/Mesh.h"
#include "components/Transform.h"

class Entity {
public:
	Entity(const std::string& name = "Entity");
	~Entity() = default;

	// per-frame logic
	void update(float deltaTime);
	void render(const glm::mat4& view, const glm::mat4& projection);

	// components
	Transform transform;
	std::shared_ptr<Shader> shader; // must be set externally
	std::vector<Mesh> meshes;
	// TODO: add more components here

	std::string name;
};