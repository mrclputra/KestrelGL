#define STB_IMAGE_IMPLEMENTATION
#include "App.hpp"

static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
  auto app = static_cast<App*>(glfwGetWindowUserPointer(window));
  app->onFrameBufferSize(width, height);
}
static void cursor_position_callback(GLFWwindow* window, double xPos, double yPos) {
  ImGui_ImplGlfw_CursorPosCallback(window, xPos, yPos); // ImGui

  if (ImGui::GetIO().WantCaptureMouse) return; // if imgui wants mouse position, stop here
  
  auto app = static_cast<App*>(glfwGetWindowUserPointer(window));
  app->onCursorPos(xPos, yPos);
}
static void scroll_callback(GLFWwindow* window, double xOff, double yOff) {
  ImGui_ImplGlfw_ScrollCallback(window, xOff, yOff); // ImGui
  if (ImGui::GetIO().WantCaptureMouse) return; // if imgui wants the scroll, stop here

  auto app = static_cast<App*>(glfwGetWindowUserPointer(window));
  app->onScroll(xOff, yOff);
}
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods); // ImGui

  if (ImGui::GetIO().WantCaptureKeyboard) return; // if imgui wants the keyboard, stop here

  auto app = static_cast<App*>(glfwGetWindowUserPointer(window));
  app->onKey(key, scancode, action, mods);
}
static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
  ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods); // ImGui
  
  auto app = static_cast<App*>(glfwGetWindowUserPointer(window));
  if (ImGui::GetIO().WantCaptureMouse) return; // if imgui wants mouse button control, stop here

  if (button == GLFW_MOUSE_BUTTON_LEFT) {
    if (action == GLFW_PRESS) {
      app->mousePressed = true;
      app->firstMouse = true; // reset drag
      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // hide cursor
    }
    else if (action == GLFW_RELEASE) {
      app->mousePressed = false;
      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); // show cursor
    }
  }
}

// constructor
App::App(int w, int h, const char* t) 
  : window(nullptr), width(w), height(h), title(t) {
  init();
}
// destructor
App::~App() {}

void App::init() {
  // initialize GLFW
  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW\n";
    std::exit(EXIT_FAILURE);
  }

  // config GLFW
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

  // center window
  GLFWmonitor* primary = glfwGetPrimaryMonitor();
  const GLFWvidmode* mode = glfwGetVideoMode(primary);
  int xpos = (mode->width - width) / 2;
  int ypos = (mode->height - height) / 2;
  glfwSetWindowPos(window, xpos, ypos);

  glfwMakeContextCurrent(window);
  glfwSetWindowUserPointer(window, this);

  // load OpenGL function pointers through GLAD
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cerr << "Failed to initialize GLAD\n";
    std::exit(EXIT_FAILURE);
  }

  // initialize ImGUI
  gui.init(this, window);

  // initialize viewport
  int fbWidth, fbHeight;
  glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
  glViewport(0, 0, fbWidth, fbHeight);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  // setup OpenGL configurations here
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  camera = Camera(4.0f, 0.0f, 0.0f);

  // setup scene
  loadShaders();
  loadModel();

  setupCallbacks();
}

void App::setupCallbacks() {
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetCursorPosCallback(window, cursor_position_callback);
  glfwSetScrollCallback(window, scroll_callback);
  glfwSetKeyCallback(window, key_callback);
  glfwSetMouseButtonCallback(window, mouse_button_callback);

  // config cursor
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); // normal mode
}

void App::run() {
  float lastTime = 0.0f;
  float lastShaderCheck = 0.0f;

  // per-frame logic below
  while (!glfwWindowShouldClose(window)) {
    float currentTime = glfwGetTime();
    float deltaTime = currentTime - lastTime;
    lastTime = currentTime;

    // check shaders
    if (currentTime - lastShaderCheck > 2.0f) {
      shader.checkAndReload(); // update base model shader
      skybox.getShader().checkAndReload(); // update skybox shader
      lastShaderCheck = currentTime;
    }

    // update camera
    camera.update();

    // update model rotation
    if (rotateModel) {
      currentModelRotation += modelRotateSpeed * deltaTime;
      if (currentModelRotation >= 360.0f) {
        currentModelRotation -= 360.0f;
      }
    }

    // draw gui
    gui.beginFrame();
    gui.draw();

    if (!ImGui::GetIO().WantCaptureMouse && mousePressed) {
      double xPos, yPos;
      glfwGetCursorPos(window, &xPos, &yPos);
      onCursorPos(xPos, yPos);
    }

    // render
    //glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // TODO: move these single initialized stuff outside of per-frame loop
    // I just put these here for simplicity
    glm::mat4 viewMatrix = camera.getViewMatrix();
    glm::mat4 projectionMatrix = camera.getProjectionMatrix(width, height);

    // draw skybox
    skybox.draw(viewMatrix, projectionMatrix, camera.position);

    // draw main model
    shader.use();
    shader.setMat4("view", viewMatrix);
    shader.setMat4("projection", projectionMatrix);
    shader.setVec3("viewPos", camera.position);

    glActiveTexture(GL_TEXTURE10); // base cubemap
    glBindTexture(GL_TEXTURE_CUBE_MAP, skybox.getCubemapTexture());
    shader.setInt("skybox", 10);

    glActiveTexture(GL_TEXTURE11); // base irradiance
    glBindTexture(GL_TEXTURE_CUBE_MAP, skybox.getIrradianceMap());
    shader.setInt("irradiance", 11);
    
    // transform model
    // scale -> rotate -> translate

    glm::mat4 modelMatrix(1.0f);
    if (rotateModel) {
      modelMatrix = glm::rotate(modelMatrix, glm::radians(currentModelRotation), glm::vec3(0.0f, 1.0f, 0.0f));
    }

    modelMatrix = glm::scale(modelMatrix, glm::vec3(model.getScaleToStandard(3.0f))); // standardize scale
    
    //modelMatrix = glm::rotate(modelMatrix, glm::radians(-90.f), glm::vec3(1, 0, 0)); // maya to opengl
    //modelMatrix = glm::rotate(modelMatrix, glm::radians(-180.f), glm::vec3(1, 0, 0)); // flip 180
    
    modelMatrix = glm::translate(modelMatrix, -model.getCenter()); // center model at origin

    shader.setMat4("model", modelMatrix);
    model.Draw(shader, camera.position); // draw model

    gui.endFrame();

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  gui.shutdown();
  cleanup();
}

void App::loadShaders() {
  shader = Shader(SHADER_DIR "/model.vert", SHADER_DIR "/model.frag");
}

void App::loadModel(const char* path) {
  stbi_set_flip_vertically_on_load(true);
  model = Model(path); // update model

  //camera.reset();
}

// loads default model
void App::loadModel() {
  stbi_set_flip_vertically_on_load(true);
  model = Model("assets/models/sphere.obj");

  skyboxSets = {
    "assets/skybox/farmland_overcast_4k.hdr",
    "assets/skybox/evening_field_4k.hdr",
    "assets/skybox/drackenstein_quarry_puresky_4k.hdr"
  };

  loadCurrentSkybox();
}

void App::loadNextSkybox() {
  currentSkyboxIndex = (currentSkyboxIndex + 1) % skyboxSets.size();
  loadCurrentSkybox();
}

void App::loadPreviousSkybox() {
  currentSkyboxIndex = (currentSkyboxIndex - 1 + skyboxSets.size()) % skyboxSets.size();
  loadCurrentSkybox();
}

void App::loadCurrentSkybox() {
  if (currentSkyboxIndex >= 0 && currentSkyboxIndex < skyboxSets.size()) {
    skybox.load(skyboxSets[currentSkyboxIndex]);
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
  if (!mousePressed) return;

  if (firstMouse) {
    lastX = xPos;
    lastY = yPos;
    firstMouse = false;
  }

  float xOffset = xPos - lastX;
  float yOffset = yPos - lastY;
  lastX = xPos;
  lastY = yPos;

  camera.rotate(xOffset, yOffset);

  // wrap cursor
  int w, h;
  glfwGetWindowSize(window, &w, &h);

  bool wrapped = false;
  if (xPos <= 0) { xPos = w - 2; wrapped = true; }
  if (xPos >= w - 1) { xPos = 1; wrapped = true; }
  if (yPos <= 0) { yPos = h - 2; wrapped = true; }
  if (yPos >= h - 1) { yPos = 1; wrapped = true; }

  if (wrapped) {
    glfwSetCursorPos(window, xPos, yPos);
    lastX = xPos;
    lastY = yPos;
  }
}

void App::onScroll(double xOff, double yOff) {
  camera.zoom(yOff);
}

void App::onKey(int key, int scancode, int action, int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
}