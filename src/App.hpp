#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <camera.hpp>
#include <model.hpp>
#include <shader.hpp>

struct Light {
  glm::vec3 position;
  glm::vec3 color;
};

class App {
public:
  App(int width, int height, const char* title);
  ~App();

  void run();

  // callback handlers
  void onFrameBufferSize(int width, int height);
  void onCursorPos(double xPos, double yPos);
  void onScroll(double xOff, double yOff);
  void onKey(int key, int scancode, int action, int mods);

  bool firstMouse = true;
  bool mousePressed = false;

private:
  void init();
  void setupCallbacks();
  void cleanup();

  void loadShaders();
  void loadModel();
  void setupLighting();
  void update(); // if any per-frame updates

  GLFWwindow* window;
  int width, height;
  const char* title;

  Camera camera;
  float lastX = 0.0f;
  float lastY = 0.0f;

  Shader shader;
  Model model;
  std::vector<Light> lights;

  float deltaTime = 0.0f;
  float lastFrame = 0.0f;
};