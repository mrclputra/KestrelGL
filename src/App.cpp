#include "App.hpp"
#include <iostream>

static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
  auto app = static_cast<App*>(glfwGetWindowUserPointer(window));
  app->onFrameBufferSize(width, height);
}
static void cursor_position_callback(GLFWwindow* window, double xPos, double yPos) {
  auto app = static_cast<App*>(glfwGetWindowUserPointer(window));
  app->onCursorPos(xPos, yPos);
}
static void scroll_callback(GLFWwindow* window, double xOff, double yOff) {
  auto app = static_cast<App*>(glfwGetWindowUserPointer(window));
  app->onScroll(xOff, yOff);
}
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  auto app = static_cast<App*>(glfwGetWindowUserPointer(window));
  app->onKey(key, scancode, action, mods);
}

App::App(int w, int h, const char* t) 
  : window(nullptr), width(w), height(h), title(t) {
  init();
  setupCallbacks();
}

App::~App() {
  cleanup();
}

void App::init() {
  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW\n";
    std::exit(EXIT_FAILURE);
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // create a window
  window = glfwCreateWindow(width, height, title, nullptr, nullptr);
  if (window == NULL) {
    std::cerr << "Failed to create GLFW window\n";
    glfwTerminate();
    std::exit(EXIT_FAILURE);
  }

  glfwMakeContextCurrent(window);
  glfwSetWindowUserPointer(window, this);

  // load OpenGL function pointers through GLAD
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cerr << "Failed to initialize GLAD\n";
    std::exit(EXIT_FAILURE);
  }

  // initial viewport
  int fbWidth, fbHeight;
  glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
  glViewport(0, 0, fbWidth, fbHeight);
}

void App::setupCallbacks() {
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetCursorPosCallback(window, cursor_position_callback);
  glfwSetScrollCallback(window, scroll_callback);
  glfwSetKeyCallback(window, key_callback);

  // config cursor
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); // set to disabled
}

void App::run() {
  while (!glfwWindowShouldClose(window)) {
    // render
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // TODO: draw scene

    glfwSwapBuffers(window);
    glfwPollEvents();
  }
}

void App::cleanup() {
  if (window) {
    glfwDestroyWindow(window);
    window = nullptr;
  }
  glfwTerminate();
}

void App::onFrameBufferSize(int w, int h) {
  glViewport(0, 0, w, h);
  width = w;
  height = h;
}

void App::onCursorPos(double xPos, double yPos) {
  // TODO: handle cursor movement
}

void App::onScroll(double xOff, double yOff) {
  // TODO: handle scroll (yOff)
}

void App::onKey(int key, int scancode, int action, int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
}