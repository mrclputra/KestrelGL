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
    ImGui::Text("rad   : %.2f", cam.radius);
    ImGui::Text("theta : %.2f", cam.theta);
    ImGui::Text("phi   : %.2f", cam.phi);

    ImGui::Separator();

    // global render configs
    ImGui::Text("Global Render Configuration");

    static int mode = 0;
    if (mode < 0) mode = 0;

    ImGui::Text("Mode");
    ImGui::SameLine();
    if (ImGui::Button("-")) { mode--; }
    ImGui::SameLine();
    ImGui::Text("%d", mode);
    ImGui::SameLine();
    if (ImGui::Button("+")) { mode++; }

    app->renderer.renderMode = mode;

    ImGui::Spacing();

    // irradiance

    // shadow maps!
    static bool showShadowMaps = false;
    ImGui::Checkbox("L-Depth Maps", &showShadowMaps);

    if (showShadowMaps) {
        ImGui::Begin("##Shadow Maps", &showShadowMaps,
            ImGuiWindowFlags_NoCollapse);

        for (auto& light : app->scene->lights) {
            if (auto dir = std::dynamic_pointer_cast<DirectionalLight>(light)) {
                ImGui::Text("Directional Light");
                ImGui::Image(
                    (void*)(intptr_t)dir->depthMap,
                    ImVec2(128, 128),
                    ImVec2(0, 1),
                    ImVec2(1, 0)
                );
            }
        }

        ImGui::End();
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
                        ImGui::DragFloat3("##Pos", &obj->transform.position[0], 0.1f);
                        ImGui::TreePop();
                    }

                    // shader
                    if (obj->shader && ImGui::TreeNodeEx("Shader", subFlags)) {
                        ImGui::Text("ID: %u", obj->shader->ID);
                        ImGui::TreePop();
                    }

                    // meshes
                    if (!obj->meshes.empty() && ImGui::TreeNodeEx("Meshes", subFlags)) {
                        for (int i = 0; i < (int)obj->meshes.size(); i++) {
                            ImGui::BulletText("Mesh %d", i);
                        }
                        ImGui::TreePop();
                    }

                    // pbr parameters
                    if (ImGui::TreeNodeEx("PBR", subFlags)) {
                        ImGui::Text("Metalness");
                        ImGui::SetNextItemWidth(140.0f);
                        ImGui::SliderFloat("##MetalnessFac", &obj->metalnessFac, 0.0f, 1.0f);

                        ImGui::Text("Roughness");
                        ImGui::SetNextItemWidth(140.0f);
                        ImGui::SliderFloat("##RougnessFac", &obj->roughnessFac, 0.0f, 1.0f);

                        ImGui::TreePop();
                    }

                    ImGui::TreePop();
                }
            }

            // pop objects
            ImGui::TreePop();
        }

        // lights
        if (ImGui::TreeNodeEx("Lights")) {
            for (int i = 0; i < app->scene->lights.size(); i++) {
                auto& light = app->scene->lights[i];

                // make a unique ID for the light node
                // should probably implement a proper global ID system for the engine down the line
                char lightLabel[64];
                auto dirLight = std::dynamic_pointer_cast<DirectionalLight>(light);
                bool isDir = (dirLight != nullptr);
                sprintf(lightLabel, "%s ##%p", isDir ? "Directional Light" : "Point Light", (void*)light.get());

                if (ImGui::TreeNodeEx(lightLabel, ImGuiTreeNodeFlags_OpenOnArrow)) {
                    ImGuiTreeNodeFlags subFlags = ImGuiTreeNodeFlags_DefaultOpen;

                    // appearance
                    if (ImGui::TreeNodeEx("Appearance", subFlags)) {
                        ImGui::Text("Light Color");
                        ImGui::SetNextItemWidth(100.0f);
                        ImGui::ColorEdit3("##Color", &light->color[0]);
                        ImGui::TreePop();
                    }

                    // transforms
                    if (ImGui::TreeNodeEx("Transform", subFlags)) {
                        if (isDir) {
                            ImGui::Text("Direction");
                            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "X: %.2f", dirLight->direction.x);
                            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Y: %.2f", dirLight->direction.y);
                            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Z: %.2f", dirLight->direction.z);
                        }
                        else {
                            // for other light types down the line
                            ImGui::TextDisabled("oops nothing here");
                        }

                        ImGui::TreePop();
                    }

                    ImGui::TreePop(); // close light instance
                }
            }

            // pop lights
            ImGui::TreePop();
        }

        // pop scene
        ImGui::TreePop();
    }

    ImGui::End();
}