#include "Gui.h"
#include "App.h"

void Gui::init(App* appPtr, GLFWwindow* window) {
    if (active) return;

    app = appPtr;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, false);
    ImGui_ImplOpenGL3_Init("#version 460");

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

    // define gui elements here
    ImGui::SetNextWindowSize(ImVec2(200, 100));
    ImGui::Begin("Debug");

    // TODO: add elements here
    auto cam = app->scene->camera;
    ImGui::Text("Camera Position");
    ImGui::Text("%.2fx, %.2fy, %.2fz,", cam.position.x, cam.position.y, cam.position.z);
    ImGui::Text("%.2fr, %.2ft, %.2fp,", cam.radius, cam.theta, cam.phi);

    ImGui::End();
}