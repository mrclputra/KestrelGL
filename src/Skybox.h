#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
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

	std::vector<glm::vec3> shCoefficients;

private:
	unsigned int m_CubemapID;
	unsigned int m_SkyboxVAO;
	unsigned int m_SkyboxVBO;

	void setupGeometry();
	unsigned int convertHDRItoCubemap(const std::string& path);
	
	// reuse the cubemapID, to generate the irradiance map
	void computeIrradiance(); 

	// shaders
	std::shared_ptr<Shader> m_SkyboxShader;		// this is for rendering
	std::shared_ptr<Shader> m_EquiToCubeShader;	// this is for conversion
};