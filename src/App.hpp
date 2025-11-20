#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>

#include <camera.hpp>
#include <model.hpp>
#include <shader.hpp>

#include "gui/Gui.hpp"

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

  bool rotateLights = false;
  float lightRotateSpeed = 0.4f;

  Camera camera;
  Model model;
  Shader shader;
  std::vector<Light> lights;

  void loadModel(const char* path);

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

  // cursor position
  float lastX = 0.0f;
  float lastY = 0.0f;

  Gui gui;

  float deltaTime = 0.0f;
  float lastFrame = 0.0f;
};