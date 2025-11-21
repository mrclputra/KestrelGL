#define STB_IMAGE_IMPLEMENTATION
#include "App.hpp"

static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
  auto app = static_cast<App*>(glfwGetWindowUserPointer(window));
  app->onFrameBufferSize(width, height);
}
static void cursor_position_callback(GLFWwindow* window, double xPos, double yPos) {
  ImGui_ImplGlfw_CursorPosCallback(window, xPos, yPos); // ImGui
  
  auto app = static_cast<App*>(glfwGetWindowUserPointer(window));
  app->onCursorPos(xPos, yPos);
}
static void scroll_callback(GLFWwindow* window, double xOff, double yOff) {
  ImGui_ImplGlfw_ScrollCallback(window, xOff, yOff); // ImGui
  
  auto app = static_cast<App*>(glfwGetWindowUserPointer(window));
  app->onScroll(xOff, yOff);
}
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  auto app = static_cast<App*>(glfwGetWindowUserPointer(window));
  app->onKey(key, scancode, action, mods);
}
static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
  ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods); // ImGui
  
  auto app = static_cast<App*>(glfwGetWindowUserPointer(window));
  if (ImGui::GetIO().WantCaptureMouse) return;

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

App::App(int w, int h, const char* t) 
  : window(nullptr), width(w), height(h), title(t) {
  init();
}

App::~App() {
  gui.shutdown();
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

  // initialize Gui
  gui.init(this, window);

  // initial viewport
  int fbWidth, fbHeight;
  glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
  glViewport(0, 0, fbWidth, fbHeight);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  //camera = Camera(10.0f, 0.0f, 0.0f);
  camera = Camera(4.0f, 0.0f, 0.0f);

  loadShaders();
  loadModel();
  setupLighting();

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
  //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); 
}

void App::run() {
  float lastTime = 0.0f;
  float lastShaderCheck = 0.0f;

  while (!glfwWindowShouldClose(window)) {
    float currentTime = glfwGetTime();
    float deltaTime = currentTime - lastTime;
    lastTime = currentTime;

    // check shaders
    if (currentTime - lastShaderCheck > 2.0f) {
      shader.checkAndReload();
      lastShaderCheck = currentTime;
    }

    update();
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

    // TODO: move single initialized stuff outside of per-frame loop
    glm::mat4 viewMatrix = camera.getViewMatrix();
    glm::mat4 projectionMatrix = camera.getProjectionMatrix(width, height);

    // draw 3d scene
    shader.use();
    shader.setMat4("view", viewMatrix);
    shader.setMat4("projection", projectionMatrix);
    shader.setVec3("viewPos", camera.position);

    shader.setInt("numLights", lights.size());
    for (int i = 0; i < (int)lights.size(); i++) {
      shader.setVec3("lights[" + std::to_string(i) + "].position", lights[i].position);
      shader.setVec3("lights[" + std::to_string(i) + "].color", lights[i].color);
    }

    glm::mat4 modelMatrix(1.0f);
    //modelMatrix = glm::scale(modelMatrix, glm::vec3(0.3f));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(model.getScaleToStandard(3.0f))); // standardize scale
    //modelMatrix = glm::rotate(modelMatrix, glm::radians(-90.f), glm::vec3(1, 0, 0)); // maya to opengl
    //modelMatrix = glm::rotate(modelMatrix, glm::radians(-180.f), glm::vec3(1, 0, 0)); // flip 180
    modelMatrix = glm::translate(modelMatrix, -model.getCenter()); // center model at origin
    shader.setMat4("model", modelMatrix);

    model.Draw(shader, camera.position);
    
    gui.endFrame();

    glfwSwapBuffers(window);
    glfwPollEvents();
  }
}

void App::loadShaders() {
  shader = Shader(SHADER_DIR "/model.vert", SHADER_DIR "/model.frag");
}

void App::loadModel(const char* path) {
  stbi_set_flip_vertically_on_load(true);
  model = Model(path);
}

// default
void App::loadModel() {
  stbi_set_flip_vertically_on_load(true);
  //model = Model("assets/models/base/tibetan-hayagriva-18th-c-ce/source/190614_mia337_132174_402_local_64Kmesh_OBJ.obj");
  //model = Model("assets/models/base/loie_fuller_sculpture_by_joseph_kratina/scene.gltf");
  //model = Model("assets/models/base/hercules_after_francesco_da_sant_agata/scene.gltf");

  model = Model("assets/models/sphere.obj");
}

void App::setupLighting() {
  lights.clear();
  lights.push_back({ glm::vec3(30.0f, 45.0f, 30.0f), glm::vec3(1.0f, 1.0f, 0.95f) });   // key
  lights.push_back({ glm::vec3(-20.0f, 20.0f, 20.0f), glm::vec3(0.7f, 0.7f, 0.75f) });  // fill
  lights.push_back({ glm::vec3(0.0f, 30.0f, -30.0f), glm::vec3(0.6f, 0.6f, 0.65f) });   // back
}

// put per-frame update stuff here
void App::update() {
  camera.update();

  if (rotateLights) {
    for (auto& light : lights) {
      glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightRotateSpeed), glm::vec3(0.0f, 1.0f, 0.0f));
      glm::vec4 rotatedPos = rotation * glm::vec4(light.position, 1.0f);
      light.position = glm::vec3(rotatedPos);
    }
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
  // TODO: handle scroll (yOff)
  camera.zoom(yOff);
}

void App::onKey(int key, int scancode, int action, int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
}