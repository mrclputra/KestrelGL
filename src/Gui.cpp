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
}

void Gui::draw() {
    if (!app) return;

    // data collection
    float currentDeltaTime = ImGui::GetIO().DeltaTime * 1000.0f; // convert to ms

    // float drift solution
    if (frameTimeOffset == 0) {
        frameTimeSum = 0.0f;
        for (int i = 0; i < FRAME_HIST_COUNT; i++) frameTimeSum += frameTimeHistory[i];
    }

    frameTimeSum -= frameTimeHistory[frameTimeOffset];
    frameTimeSum += currentDeltaTime;
    frameTimeHistory[frameTimeOffset] = currentDeltaTime;

    movingAverage = frameTimeSum / (float)FRAME_HIST_COUNT;
    frameTimeOffset = (frameTimeOffset + 1) % FRAME_HIST_COUNT;

    // optimization
    maxFrameTime = 0.0f;
    for (int i = 0; i < FRAME_HIST_COUNT; i++) {
        if (frameTimeHistory[i] > maxFrameTime) maxFrameTime = frameTimeHistory[i];
    }

    // find max for scaling graph
    maxFrameTime = 0.0f;
    for (int i = 0; i < FRAME_HIST_COUNT; i++) {
        if (frameTimeHistory[i] > maxFrameTime) maxFrameTime = frameTimeHistory[i];
    }

    ImGui::Begin("Debug", nullptr, ImGuiChildFlags_AlwaysAutoResize);

    // profiler
    ImGui::Text("Current: %.2f ms", currentDeltaTime);
    ImGui::Text("Average: %.2f ms (%.1f FPS)",
        movingAverage,
        (movingAverage > 0.0f) ? 1000.0f / movingAverage : 0.0f);

    ImGui::PlotLines("##FrameTimeGraph",
        frameTimeHistory,
        FRAME_HIST_COUNT,
        frameTimeOffset,
        nullptr,
        0.0f,
        maxFrameTime + 2.0f,
        ImVec2(ImGui::GetContentRegionAvail().x, 80));

    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Max: %.1fms", maxFrameTime);

    ImGui::Separator();

    // camera data
    auto cam = app->scene->camera;
    ImGui::Text("Camera Position");
    ImGui::Text("%.2fx, %.2fy, %.2fz,", cam.position.x, cam.position.y, cam.position.z);
    ImGui::Spacing();
    //ImGui::Text("rad   : %.2f", cam.radius);
    //ImGui::Text("theta : %.2f", cam.theta);
    //ImGui::Text("phi   : %.2f", cam.phi);

    ImGui::Separator();

    // global render configs
    ImGui::Text("Global Render Config");

    static int mode = 0;
    if (mode < 0) mode = 0;

    ImGui::Text("Mode");
    ImGui::SameLine();
    if (ImGui::Button("-")) { mode--; }
    ImGui::SameLine();
    ImGui::Text("%d", mode);
    ImGui::SameLine();
    if (ImGui::Button("+")) { mode++; }

    //app->renderer.renderMode = mode;

    ImGui::Spacing();

    // maybe make this recursive?
    //ImGui::Checkbox("Normal Maps", &app->renderer.isNormalEnabled);

    ImGui::Separator();
    ImGui::Text("Environment");

    if (ImGui::Button("Change HDRI")) {
        IGFD::FileDialogConfig config;
        config.path = ".";
        ImGui::SetNextWindowSize(ImVec2(720, 500));
        ImGuiFileDialog::Instance()->OpenDialog("ChooseSkyboxKey", "Choose HDR File", ".hdr", config);
    }

    // display the dialog
    if (ImGuiFileDialog::Instance()->Display("ChooseSkyboxKey")) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();

            if (app->scene->skybox) {
                app->scene->skybox->load(filePathName);
                logger.info("Loaded new skybox: " + filePathName);
            }
        }
        ImGuiFileDialog::Instance()->Close();
    }

    ImGui::Separator();

    // scene tree
    if (ImGui::TreeNodeEx("Scene Tree")) {
        // objects
        if (ImGui::TreeNodeEx("Objects")) {
            for (auto& obj : app->scene->objects) {
                if (ImGui::TreeNodeEx((void*)obj.get(), ImGuiTreeNodeFlags_OpenOnArrow, "%s", obj->name.c_str())) {
                    ImGuiTreeNodeFlags subFlags = ImGuiTreeNodeFlags_DefaultOpen;

                    // transforms
                    if (ImGui::TreeNodeEx("Transform", subFlags)) {
                        ImGui::Text("Position");
                        ImGui::SetNextItemWidth(140.0f);
                        if (ImGui::DragFloat3("##Pos", &obj->transform.position[0], 0.1f)) {
                            obj->transform.isDirty = true;
                        }
                        ImGui::TreePop();
                    }

                    // shader
                    if (obj->material->shader && ImGui::TreeNodeEx("Shader", subFlags)) {
                        ImGui::Text("ID: %u", obj->material->shader->ID);
                        ImGui::TreePop();
                    }

                    // meshes
                    if (!obj->meshes.empty() && ImGui::TreeNode("Meshes")) {
                        for (int i = 0; i < (int)obj->meshes.size(); i++) {
                            ImGui::BulletText("Mesh %d", i);
                        }
                        ImGui::TreePop();
                    }

                    // PBR parameters

                    ImGui::TreePop();
                }
            }

            // pop objects
            ImGui::TreePop();
        }

        // pop scene
        ImGui::TreePop();
    }

    ImGui::End();
}