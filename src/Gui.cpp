#include "Gui.hpp"
#include "App.hpp"

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

  //ImGui::Separator();
  //ImGui::Text("Lights");

  //ImGui::Checkbox("Rotate", &app->rotateLights);
  //ImGui::SliderFloat("Speed", &app->lightRotateSpeed, 0.0f, 1.0f, "%.2f");

  ImGui::Separator();
  ImGui::Text("Model");

  // model angle y
  ImGui::SliderFloat("Model Angle", &app->currentModelAngle, 0.0f, 360.0f, "%.1f deg");

  ImGui::Separator();
  ImGui::Text("Lights");

  // y rotation slider
  ImGui::SliderFloat("Lights Angle", &app->lightRotationAngle, 0.0f, 360.0f, "%.1f deg");

  ImGui::Separator();
  ImGui::Text("Skybox");

  // Skybox switching with arrow buttons
  ImGui::Text("Index: %d", app->getCurrentSkyboxIndex());

  // Left arrow button
  if (ImGui::ArrowButton("##skybox_left", ImGuiDir_Left)) {
    app->loadPreviousSkybox();
  }

  ImGui::SameLine();

  // Right arrow button  
  if (ImGui::ArrowButton("##skybox_right", ImGuiDir_Right)) {
    app->loadNextSkybox();
  }

  ImGui::SameLine();
  ImGui::Text("Type");

  ImGui::Separator();
  ImGui::Text("Model");

  // input buffer for model path
  static char modelPath[256] = "assets/models/base/loie_fuller_sculpture_by_joseph_kratina/scene.gltf";
  ImGui::InputText("Path", modelPath, sizeof(modelPath));

  if (ImGui::Button("Browse...")) {
    IGFD::FileDialogConfig config;
    config.path = ".";
    ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlg", "Choose Model", ".gltf,.glb,.obj,.fbx,.ply", config);
  }

  // file dialog popup
  if (ImGuiFileDialog::Instance()->Display("ChooseFileDlg")) {
    if (ImGuiFileDialog::Instance()->IsOk()) {
      std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
      strcpy(modelPath, filePathName.c_str());

      app->loadModel(modelPath); // autoload
    }
    ImGuiFileDialog::Instance()->Close();
  }

  ImGui::End();
}