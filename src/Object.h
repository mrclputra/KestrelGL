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

// TODO: make this object object work even with no shaders, meshes, or materials
class Object {
public:
	Object(const std::string& name = "Object");
	~Object() = default;

	// per-frame logic
	void update(float deltaTime);
	void render(const glm::mat4& view, const glm::mat4& projection);

	// the components below must be assigned before any draws are made
	// as such, the modelloader class is responsible for doing this

	// components, set these externally
	Transform transform;
	std::shared_ptr<Shader> shader;
	std::vector<std::shared_ptr<Mesh>> meshes;
	std::vector<std::shared_ptr<Material>> materials;
	// TODO: add more components here if needed

	// the main idea is that during the render pass,
	// we iterate through all components and set shader uniforms

	std::string name;
};