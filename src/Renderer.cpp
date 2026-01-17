#include "Renderer.h"

void Renderer::init(const Scene& scene) {
	// if we need to pre-bake anything, we do it here
}

void Renderer::render(const Scene& scene) {
	renderSkybox(scene);

	commands.clear();
	collectDrawCommands(scene);
	executeBatched(scene);
}

void Renderer::collectDrawCommands(const Scene& scene) {
	for (const auto& object : scene.objects) {
		if (!object->material->shader || object->material->shader->ID == 0) continue;

		float distance = glm::length(scene.camera.position - object->transform.position);
		glm::mat4 modelMatrix = object->transform.getModelMatrix();

		for (const auto& mesh : object->meshes) {
			commands.push_back({
				mesh.get(),
				object->material.get(),
				modelMatrix,
				distance
				});
		}
	}

	// sort
	std::sort(commands.begin(), commands.end(),
		[](const DrawCommand& a, const DrawCommand& b) {
			if (a.material->isTransparent != b.material->isTransparent) {
				return a.material->isTransparent;
			}

			return a.material->shader < b.material->shader;
		});
}

void Renderer::executeBatched(const Scene& scene) {
	if (commands.empty()) return;
	Shader* currentShader = nullptr;

	for (const auto& cmd : commands) {
		if (currentShader != cmd.material->shader.get()) {
			currentShader = cmd.material->shader.get();

			currentShader->checkHotReload();
			currentShader->use();

			currentShader->setMat4("view", scene.camera.getViewMatrix());
			currentShader->setMat4("projection", scene.camera.getProjectionMatrix());
			currentShader->setVec3("viewPos", scene.camera.position);
		}

		currentShader->setMat4("model", cmd.modelMatrix);
		currentShader->setVec4("p_albedo", cmd.material->albedo);
		currentShader->setFloat("p_metalness", cmd.material->metalness);
		currentShader->setFloat("p_roughness", cmd.material->roughness);

		currentShader->setBool("hasAlbedoMap", false);

		// textures
		for (int texIdx : cmd.mesh->texIndices) {
			const auto& tex = scene.textures[texIdx];
			
			switch (tex->type) {
			case Texture::Type::ALBEDO:
				currentShader->setBool("hasAlbedoMap", true);
				currentShader->setInt("albedoMap", 0);
				tex->bind(0);
				break;

			// albedo: 0
			// normal: 1
			// metrough: 2
			// ao = 3
			// emissive: 4
			}
		}

		cmd.mesh->render();
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