#include "Renderer.h"

void Renderer::init(const Scene& scene) {
	// if we need to pre-bake anything, we do it here
}

void Renderer::render(const Scene& scene) {
	renderSkybox(scene);

	drawOrder.clear();
	for (const auto object : scene.objects) {
		float distance = glm::length(scene.camera.position - object->transform.position);
		drawOrder.insert(std::make_pair(distance, object));
	}
	for (const auto& pair : drawOrder) {
		renderObject(scene, *pair.second);
	}
}

void Renderer::renderObject(const Scene& scene, const Object& object) {
	if (!object.material->shader|| object.material->shader->ID == 0) return;

	Shader& shader = *object.material->shader;
	shader.checkHotReload();
	shader.use();

	// object transformations
	shader.setMat4("model", object.transform.getModelMatrix());
	// camera
	shader.setMat4("view", scene.camera.getViewMatrix());
	shader.setMat4("projection", scene.camera.getProjectionMatrix());
	shader.setVec3("viewPos", scene.camera.position);

	shader.setVec4("p_albedo", object.material->albedo);
    shader.setFloat("p_metalness", object.material->metalness);
    shader.setFloat("p_roughness", object.material->roughness);

	// render textures
	for (const auto& mesh : object.meshes) {
		for (int texIdx : mesh->texIndices) {
			const auto& tex = object.material->textures[texIdx];

			switch (tex->type) {
			case Texture::Type::ALBEDO:
				shader.setBool("hasAlbedoMap", true);
				shader.setInt("albedoMap", 0);
				tex->bind(0);
				break;
			}
		}

		mesh->render();
	}
}

void Renderer::renderSkybox(const Scene& scene) {
	scene.skybox->m_SkyboxShader->checkHotReload();
	if (scene.skybox) {
		scene.skybox->draw(scene.camera.getViewMatrix(), scene.camera.getProjectionMatrix(), scene.camera.position);
	}
	else {
		logger.error("no skybox to render");
	}
}