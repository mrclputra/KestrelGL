#ifndef SKYBOX_HPP
#define SKYBOX_HPP

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <model.hpp>
#include <shader.hpp>
#include <vector>
#include <string>

class Skybox {
public:
  Skybox();
  ~Skybox();

  // load HDR file
  bool load(const std::string& hdrPath);

  void draw(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& cameraPos);

  // get the shader for external use
  // if needed
  Shader& getShader() { return shader; }
  unsigned int getCubemapTexture() const { return cubemapTexture; }
  unsigned int getIrradianceMap() const { return irradianceMap; }

  void cleanup();

private:
  unsigned int hdrToCubemap(const std::string& hdrPath);
  unsigned int generateIrradianceMap();
  void renderCube();

  Shader shader;
  unsigned int cubemapTexture;
  unsigned int irradianceMap;
  bool initialized;

  // cube VAO for conversion
  unsigned int cubeVAO;
  unsigned int cubeVBO;
};

#endif // !SKYBOX_HPP
