#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>

#include <camera.hpp>
#include <model.hpp>
#include <shader.hpp>

#include "Gui.hpp"
#include "Skybox.hpp"

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

  Camera camera;

  bool rotateModel = false;
  float modelRotateSpeed = 30.0f; // degrees per second
  float currentModelRotation = 0.0f;

  Model model;
  Skybox skybox;

  Shader shader;

  void loadModel(const char* path); // override

  // skybox switching
  void loadNextSkybox();
  void loadPreviousSkybox();
  int getCurrentSkyboxIndex() const { return currentSkyboxIndex; }

private:
  void init(); // setup
  void setupCallbacks();
  void cleanup();

  void loadShaders();
  void loadModel();

  GLFWwindow* window;
  int width, height;
  const char* title;

  // cursor position
  float lastX = 0.0f;
  float lastY = 0.0f;

  Gui gui;

  float deltaTime = 0.0f;
  float lastFrame = 0.0f;

  // skybox switching
  //std::vector<std::vector<std::string>> skyboxSets;
  std::vector<std::string> skyboxSets;
  int currentSkyboxIndex = 0;
  void loadCurrentSkybox();
};