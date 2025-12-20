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

    // set size
    ImGui::SetNextWindowSize(ImVec2(200, 300));

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
    ImGui::Begin("Debug");

    // TODO: add elements here
    auto cam = app->scene->camera;
    ImGui::Text("Camera Position");
    ImGui::Text("%.2fx, %.2fy, %.2fz,", cam.position.x, cam.position.y, cam.position.z);
    ImGui::Spacing();
    ImGui::Text("rad   : %.2f", cam.radius);
    ImGui::Text("theta : %.2f", cam.theta);
    ImGui::Text("phi   : %.2f", cam.phi);
    //ImGui::Text("%.2fr, %.2ft, %.2fp,", cam.radius, cam.theta, cam.phi);

    ImGui::End();


    // SHADOWMAP FRAMEBUFFER PREVIEW
    ImGui::Begin("L-Depth Buffers", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    for (auto light : app->scene->lights) {
        if (auto dir = std::dynamic_pointer_cast<DirectionalLight>(light)) {
            //ImGui::Text("Directional Light: %d", dir->depthMap);
            ImGui::Image((void*)(intptr_t)dir->depthMap, ImVec2(128, 128), ImVec2(0, 1), ImVec2(1, 0));
        }
    }

    ImGui::End();
}