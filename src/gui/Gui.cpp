#include "Gui.hpp"
#include "../App.hpp"

Gui::Gui() {};
Gui::~Gui() {};

void Gui::init(App* appPtr, GLFWwindow* window) {
  if (active) return;

  app = appPtr;

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  ImGuiIO& io = ImGui::GetIO(); (void)io;
  ImGui::StyleColorsDark();

  ImGui_ImplGlfw_InitForOpenGL(window, false);
  ImGui_ImplOpenGL3_Init("#version 330");

  active = true;
}

void Gui::shutdown() {
  if (!active) return;

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  active = false;
}

void Gui::beginFrame() {
  if (!active) return;

  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
}

void Gui::endFrame() {
  if (!active) return;

  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  ImGuiIO& io = ImGui::GetIO();

  // if needed, add multi-window support here
}

void Gui::draw() {
  if (!app) return;

  ImGui::Begin("Debug");

  ImGui::Text("Camera");
  ImGui::DragFloat3("Position", &app->camera.position[0], 0.1f);

  ImGui::Separator();
  ImGui::Text("Lights");

  ImGui::Checkbox("Rotate", &app->rotateLights);
  ImGui::SliderFloat("Speed", &app->lightRotateSpeed, 0.0f, 1.0f, "%.2f");

  ImGui::End();
}