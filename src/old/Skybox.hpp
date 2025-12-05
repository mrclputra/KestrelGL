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
  struct SHCoefficients {
    glm::vec3 L00, L1_1, L10, L11;  // l=0 and l=1 (4 coefficients)
    glm::vec3 L2_2, L2_1, L20, L21, L22;  // l=2 (5 coefficients)
  };
  SHCoefficients shCoeffs;

  SHCoefficients computeSHCoefficients(unsigned int cubemap);

  unsigned int hdrToCubemap(const std::string& hdrPath);
  unsigned int generateIrradianceMapFromSH();
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
