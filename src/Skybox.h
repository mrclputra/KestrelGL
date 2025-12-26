#pragma once

#include <glm/glm.hpp>
#include <string>
#include <memory>
#include <vector>

#include "components/Shader.h"
#include "logger.h"

class Skybox {
public:
	Skybox();
	~Skybox();

	void load(const std::string& path);
	void draw(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& cameraPos);

private:
	unsigned int m_CubemapID;
	unsigned int m_SkyboxVAO;
	unsigned int m_SkyboxVBO;

	void setupGeometry();
	unsigned int convertHDRItoCubemap(const std::string& path);

	// shaders
	std::shared_ptr<Shader> m_SkyboxShader;		// this is for rendering
	std::shared_ptr<Shader> m_EquiToCubeShader;	// this is for conversion
};