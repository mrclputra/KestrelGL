#include "Renderer.h"

void Renderer::render(const Scene& scene) {
	//const auto& view = scene.camera.getViewMatrix();
	//const auto& proj = scene.camera.getProjectionMatrix();

	for (const auto& objPtr : scene.objects) {
		renderObject(scene, *objPtr);
	}
}

void Renderer::renderObject(const Scene& scene, const Object& object) {
	Shader& shader = *object.shader;
	
	shader.checkHotReload();
	shader.use();

	shader.setMat4("model", object.transform.getModelMatrix());
	shader.setMat4("view", scene.camera.getViewMatrix());
	shader.setMat4("projection", scene.camera.getProjectionMatrix());
	shader.setVec3("viewPos", scene.camera.position);

	uploadLights(scene, shader);

	// textures
	unsigned int slot = 0;
	for (const auto& tex : object.textures) {
		tex->bind(slot);
		switch (tex->type) {
			case Texture::Type::ALBEDO: shader.setInt("albedoMap", slot); break;
			case Texture::Type::NORMAL: shader.setInt("normalMap", slot); break;
			case Texture::Type::METALLIC_ROUGHNESS: shader.setInt("metallicRoughnessMap", slot); break;
			case Texture::Type::OCCLUSION: shader.setInt("aoMap", slot); break;
			case Texture::Type::EMISSION: shader.setInt("emissionMap", slot); break;
		}
		++slot;
	}

	for (const auto& mesh : object.meshes) {
		mesh->render();
	}
}

void Renderer::uploadLights(const Scene& scene, Shader& shader) {
	constexpr int MAX_LIGHTS = 8;
	int count = std::min<int>(scene.lights.size(), MAX_LIGHTS);

	for (int i = 0; i < count; ++i) {
		auto& light = scene.lights[i];
		std::string base = "lights[" + std::to_string(i) + "]";

		shader.setVec3(base + ".color", light->color);

		if (auto dir = std::dynamic_pointer_cast<DirectionalLight>(light)) {
			shader.setVec3(base + ".position", glm::vec3(0.0f));
			shader.setVec3(base + ".direction", dir->direction);
			shader.setInt(base + ".type", 0);
		}
		else if (auto point = std::dynamic_pointer_cast<PointLight>(light)) {
			shader.setVec3(base + ".position", point->transform.position);
			shader.setVec3(base + ".direction", glm::vec3(0.0f));
			shader.setInt(base + ".type", 1);
		}
	}

	shader.setInt("numLights", count);
}