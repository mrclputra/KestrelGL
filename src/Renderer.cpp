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

	// TODO: group by same texture

	// sort
	std::sort(commands.begin(), commands.end(),
		[](const DrawCommand& a, const DrawCommand& b) {
			if (a.material->isTransparent != b.material->isTransparent) {
				return a.material->isTransparent;
			}

			if (a.material->shader != b.material->shader) {
				return a.material->shader < b.material->shader;
			}

			if (!a.mesh->texIndices.empty() && !b.mesh->texIndices.empty()) {
				return a.mesh->texIndices[0] < b.mesh->texIndices[0];
			}

			return false;
		});
}

void Renderer::executeBatched(const Scene& scene) {
	if (commands.empty()) return;

	// TODO: resolve the dereference pointer call
	Shader* shader = nullptr;

	for (const auto& cmd : commands) {
		if (!cmd.material || !cmd.material->shader) continue;

		if (shader != cmd.material->shader.get()) {
			// shader switching logic
			// the idea behind this is we only switch the shader only when we need to
			shader = cmd.material->shader.get();

			shader->checkHotReload();
			shader->use();

			shader->setMat4("view", scene.camera.getViewMatrix());
			shader->setMat4("projection", scene.camera.getProjectionMatrix());
			shader->setVec3("viewPos", scene.camera.position);
		}

		shader->setMat4("model", cmd.modelMatrix);
		shader->setVec4("p_albedo", cmd.material->albedo);
		shader->setFloat("p_metalness", cmd.material->metalness);
		shader->setFloat("p_roughness", cmd.material->roughness);

		shader->setBool("hasAlbedoMap", false);

		// textures
		for (int texIdx : cmd.mesh->texIndices) {
			const auto& tex = scene.textures[texIdx];
			
			switch (tex->type) {
			case Texture::Type::ALBEDO:
				shader->setBool("hasAlbedoMap", true);
				shader->setInt("albedoMap", 0);
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