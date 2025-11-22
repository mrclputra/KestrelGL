#include "Skybox.hpp"
#include <stb_image.h>
#include <iostream>

Skybox::Skybox() : cubemapTexture(0), initialized(false) {}
Skybox::~Skybox() { cleanup(); }

Shader& Skybox::getShader() { return shader; }

bool Skybox::load(const std::string& modelPath, const std::vector<std::string>& faces) {
  try {
    // load shaders
    shader = Shader(SHADER_DIR "/skybox.vert", SHADER_DIR "/skybox.frag");

    // load geometry
    stbi_set_flip_vertically_on_load(true);
    model = Model(modelPath);

    // load cubemap texture
    cubemapTexture = loadCubemap(faces);

    initialized = true;
    std::cout << "Skybox Loaded.\n";
    return true;
  }
  catch (const std::exception& e) {
    std::cerr << "Failed to load skybox: " << e.what() << "\n";
    return false;
  }
}

void Skybox::draw(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& cameraPos) {
  if (!initialized) return;

  glDepthMask(GL_FALSE);
  shader.use();

  // bind cubemap texture
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
  shader.setInt("skybox", 0);

  glm::mat4 skyboxView = glm::mat4(glm::mat3(view)); // remove translation from skybox
  shader.setMat4("view", skyboxView);
  shader.setMat4("projection", projection);
  shader.setMat4("model", glm::mat4(1.0f));

  model.Draw(shader, cameraPos, false); // draw
  glDepthMask(GL_TRUE);
}

unsigned int Skybox::loadCubemap(const std::vector<std::string>& faces) {
  unsigned int textureID;
  glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

  stbi_set_flip_vertically_on_load(false); // disable flipping

  int width, height, nrChannels;
  for (unsigned int i = 0; i < faces.size(); i++) {
    unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
    if (data) {
      GLenum format = GL_RGB;
      if (nrChannels == 1) format = GL_RED;
      else if (nrChannels == 3) format = GL_RGB;
      else if (nrChannels == 4) format = GL_RGBA;

      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format,
        width, height, 0, format, GL_UNSIGNED_BYTE, data);
      stbi_image_free(data);

      std::cout << "Loaded skybox face: " << faces[i] << std::endl;
    }
    else {
      std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
      stbi_image_free(data);
    }
  }

  stbi_set_flip_vertically_on_load(true); // restore flip

  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  return textureID;
}

void Skybox::cleanup() {
  if (cubemapTexture != 0) {
    glDeleteTextures(1, &cubemapTexture);
    cubemapTexture = 0;
  }
  initialized = false;
}