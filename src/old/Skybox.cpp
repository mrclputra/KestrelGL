#include "Skybox.hpp"
#include <stb_image.h>
#include <iostream>

Skybox::Skybox() : cubemapTexture(0), irradianceMap(0), initialized(false), cubeVAO(0), cubeVBO(0) {}
Skybox::~Skybox() { cleanup(); }

bool Skybox::load(const std::string& hdrPath) {
  try {
    shader = Shader(SHADER_DIR "/skybox.vert", SHADER_DIR "/skybox.frag");

    // load skyboox texture
    stbi_set_flip_vertically_on_load(true);
    cubemapTexture = hdrToCubemap(hdrPath);

    // generate diffuse irradiance map
    shCoeffs = computeSHCoefficients(cubemapTexture);
    irradianceMap = generateIrradianceMapFromSH();

    // generate specular map


    initialized = true;

    std::cout << "Skybox loaded from: " << hdrPath << "\n";
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
  shader.setInt("skybox", 0); // base texture

  glm::mat4 skyboxView = glm::mat4(glm::mat3(view));
  shader.setMat4("view", skyboxView);
  shader.setMat4("projection", projection);

  renderCube();
  glDepthMask(GL_TRUE);
}

unsigned int Skybox::hdrToCubemap(const std::string& hdrPath) {
  //stbi_set_flip_vertically_on_load(false);

  // load HDR
  int width, height, nrChannels;
  float* data = stbi_loadf(hdrPath.c_str(), &width, &height, &nrChannels, 0);

  if (!data) {
    std::cerr << "Failed to load HDR: " << hdrPath << "\n";
    //stbi_set_flip_vertically_on_load(true);
    return 0;
  }

  // create equirectangular texture
  unsigned int hdrTexture;
  glGenTextures(1, &hdrTexture);
  glBindTexture(GL_TEXTURE_2D, hdrTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);
  
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  stbi_image_free(data);
  //stbi_set_flip_vertically_on_load(true);

  // create cubemap
  unsigned int cubemap;
  glGenTextures(1, &cubemap);
  glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);

  for (unsigned int i = 0; i < 6; i++) {
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F,
      2048, 2048, 0, GL_RGB, GL_FLOAT, nullptr);
  }

  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // save current viewport
  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);

  // setup conversion shader and framebuffer
  Shader equirectToCube(SHADER_DIR "/cubemap.vert", SHADER_DIR "/equirect_to_cube.frag");

  unsigned int captureFBO, captureRBO;
  glGenFramebuffers(1, &captureFBO);
  glGenRenderbuffers(1, &captureRBO);
  glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
  glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 2048, 2048);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

  // projection for capturing each face
  glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
  glm::mat4 captureViews[] = {
    glm::lookAt(glm::vec3(0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
    glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
    glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
    glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
    glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
    glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
  };

  // convert HDR to cubemap
  equirectToCube.use();
  equirectToCube.setInt("equirectangularMap", 0);
  equirectToCube.setMat4("projection", captureProjection);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, hdrTexture);

  glViewport(0, 0, 2048, 2048);
  glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);

  for (unsigned int i = 0; i < 6; i++) {
    equirectToCube.setMat4("view", captureViews[i]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
      GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, cubemap, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    renderCube();
  }

  // restore previous state
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

  // cleanup
  glDeleteTextures(1, &hdrTexture);
  glDeleteFramebuffers(1, &captureFBO);
  glDeleteRenderbuffers(1, &captureRBO);

  return cubemap;
}

unsigned int Skybox::generateIrradianceMapFromSH() {
  // create output cubemap
  unsigned int irradianceMap;
  glGenTextures(1, &irradianceMap);
  glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);

  for (unsigned int i = 0; i < 6; i++) {
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F,
      64, 64, 0, GL_RGB, GL_FLOAT, nullptr);
  }

  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // setup framebuffer
  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);

  unsigned int captureFBO, captureRBO;
  glGenFramebuffers(1, &captureFBO);
  glGenRenderbuffers(1, &captureRBO);
  glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
  glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 64, 64);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
    GL_RENDERBUFFER, captureRBO);

  // use shader
  Shader shShader(SHADER_DIR "/cubemap.vert", SHADER_DIR "/irradiance_sh.frag");

  shShader.use();

  // upload the 9 SH coefficients to shader
  shShader.setVec3("L00", shCoeffs.L00);
  shShader.setVec3("L1_1", shCoeffs.L1_1);
  shShader.setVec3("L10", shCoeffs.L10);
  shShader.setVec3("L11", shCoeffs.L11);
  shShader.setVec3("L2_2", shCoeffs.L2_2);
  shShader.setVec3("L2_1", shCoeffs.L2_1);
  shShader.setVec3("L20", shCoeffs.L20);
  shShader.setVec3("L21", shCoeffs.L21);
  shShader.setVec3("L22", shCoeffs.L22);

  glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
  shShader.setMat4("projection", captureProjection);

  glViewport(0, 0, 64, 64);
  glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);

  glm::mat4 captureViews[] = {
    glm::lookAt(glm::vec3(0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
    glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
    glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
    glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
    glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
    glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
  };

  for (unsigned int i = 0; i < 6; i++) {
    shShader.setMat4("view", captureViews[i]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
      GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    renderCube();
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

  glDeleteFramebuffers(1, &captureFBO);
  glDeleteRenderbuffers(1, &captureRBO);

  return irradianceMap;
}

// TODO: generate specular IBL
// TODO: BRDF LUT

void Skybox::renderCube() {
  if (cubeVAO == 0) {
    float vertices[] = {
      -1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f,
       1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,
      -1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
       1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f,
      -1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,
      -1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f,
       1.0f,  1.0f,  1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f, -1.0f,
       1.0f, -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
      -1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,
       1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f, -1.0f,
      -1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,
       1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f
    };

    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindVertexArray(cubeVAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  }

  glBindVertexArray(cubeVAO);
  glDrawArrays(GL_TRIANGLES, 0, 36);
  glBindVertexArray(0); // release
}

Skybox::SHCoefficients Skybox::computeSHCoefficients(unsigned int cubemap) {
  SHCoefficients coeffs = {};

  glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);

  int actualSize;
  glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_TEXTURE_WIDTH, &actualSize);

  // read pixels from the cubemap and compute weighted sums
  // equation 10 from https://cseweb.ucsd.edu/~ravir/papers/envmap/envmap.pdf

  const int sampleSize = 64; // sample resolution per face
  int step = actualSize / sampleSize;

  std::vector<float> pixels(actualSize * actualSize * 3); // RGB

  // view directions
  glm::vec3 directions[6][3] = {
    {{0,0,-1}, {0,-1,0}, {1,0,0}},   // +X
    {{0,0,1},  {0,-1,0}, {-1,0,0}},  // -X
    {{1,0,0},  {0,0,1},  {0,1,0}},   // +Y
    {{1,0,0},  {0,0,-1}, {0,-1,0}},  // -Y
    {{1,0,0},  {0,-1,0}, {0,0,1}},   // +Z
    {{-1,0,0}, {0,-1,0}, {0,0,-1}}   // -Z
  };

  // process each face
  for (int face = 0; face < 6; face++) {
    glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, GL_RGB, GL_FLOAT, pixels.data());

    glm::vec3 right = directions[face][0];
    glm::vec3 up = directions[face][1];
    glm::vec3 forward = directions[face][2];

    // sample each pixel
    for (int y = 0; y < actualSize; y += step) {
      for (int x = 0; x < actualSize; x += step) {
        // map pixel to -1,1
        float u = (2.0f * (x + 0.5f) / actualSize) - 1.0f;
        float v = (2.0f * (y + 0.5f) / actualSize) - 1.0f;

        // get world space direction
        glm::vec3 dir = glm::normalize(forward + u * right + v * up);
        float x_dir = dir.x, y_dir = dir.y, z_dir = dir.z;

        // get pixel color
        int idx = (y * actualSize + x) * 3;
        glm::vec3 color(pixels[idx], pixels[idx + 1], pixels[idx + 2]);

        // weight by solid angle
        // accounts for cubemap distortion
        float temp = 1.0f + u * u + v * v;
        float weight = 4.0f / (sqrtf(temp) * temp);
        color *= weight;

        // accumulate into SH coefficients
        // equation 3
        // these are the Y_lm spherical harmonic basis functions

        // l=0, m=0 (constant)
        coeffs.L00 += color * 0.282095f;

        // l = 1, m = -1, 0, 1 (linear in x, y, z)
        coeffs.L1_1 += color * 0.488603f * y_dir;
        coeffs.L10 += color * 0.488603f * z_dir;
        coeffs.L11 += color * 0.488603f * x_dir;

        // l=2, m=-2,-1,0,1,2 (quadratic)
        coeffs.L2_2 += color * 1.092548f * (x_dir * y_dir);
        coeffs.L2_1 += color * 1.092548f * (y_dir * z_dir);
        coeffs.L20 += color * 0.315392f * (3.0f * z_dir * z_dir - 1.0f);
        coeffs.L21 += color * 1.092548f * (x_dir * z_dir);
        coeffs.L22 += color * 0.546274f * (x_dir * x_dir - y_dir * y_dir);
      }
    }
  }

  // normalize by samples taken
  float exposure = 0.12f;
  float normFactor = (4.0f * 3.14159265359f) / (6.0f * sampleSize * sampleSize);
  coeffs.L00 *= normFactor * exposure;
  coeffs.L1_1 *= normFactor * exposure;
  coeffs.L10 *= normFactor * exposure;
  coeffs.L11 *= normFactor * exposure;
  coeffs.L2_2 *= normFactor * exposure;
  coeffs.L2_1 *= normFactor * exposure;
  coeffs.L20 *= normFactor * exposure;
  coeffs.L21 *= normFactor * exposure;
  coeffs.L22 *= normFactor * exposure;

  //std::cout << "Irradiance SH coefficients computed from " << sampleSize << "x" << sampleSize << " cubemap\n";
  //std::cout << "Coeff L00: " << coeffs.L00.r << ", " << coeffs.L00.g << ", " << coeffs.L00.b << "\n";

  return coeffs;
}

void Skybox::cleanup() {
  if (cubemapTexture != 0) {
    glDeleteTextures(1, &cubemapTexture);
    cubemapTexture = 0;
  }
  if (cubeVAO != 0) {
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteBuffers(1, &cubeVBO);
    cubeVAO = cubeVBO = 0;
  }
  initialized = false;
}