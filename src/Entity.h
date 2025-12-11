#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <string>

#include <logger.h>
#include <shader.h>

#include "components/Mesh.h"
#include "components/Material.h"
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
	std::vector<Material> materials;
	// TODO: add more components here

	// the idea is that in the render function, we pass everything we need into the shader
	// material and texture data is passed through uniforms
	// and when all is done, we iterate through all components and render meshes

	std::string name;
};