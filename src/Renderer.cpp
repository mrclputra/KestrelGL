#include "Renderer.h"

void Renderer::render(const Scene& scene) {
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

    int dirCount = 0;
    int pointCount = 0;
    int spotCount = 0;

    for (auto& light : scene.lights) {
        if (auto dir = std::dynamic_pointer_cast<DirectionalLight>(light)) {
            if (dirCount >= MAX_LIGHTS) continue;
            std::string base = "dirLights[" + std::to_string(dirCount) + "]";
            shader.setVec3(base + ".direction", dir->direction);
            shader.setVec3(base + ".color", dir->color);
            dirCount++;
        }
        else if (auto point = std::dynamic_pointer_cast<PointLight>(light)) {
            if (pointCount >= MAX_LIGHTS) continue;
            std::string base = "pointLights[" + std::to_string(pointCount) + "]";
            shader.setVec3(base + ".position", point->transform.position);
            shader.setVec3(base + ".color", point->color);
            shader.setFloat(base + ".radius", point->radius);
            pointCount++;
        }
    }

    shader.setInt("numDirLights", dirCount);
    shader.setInt("numPointLights", pointCount);
    shader.setInt("numSpotLights", spotCount);
}