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

    // SCENE TREE
    ImGui::Begin("Scene TREE", nullptr, ImGuiChildFlags_AlwaysAutoResize);
    // root scene node
    if (ImGui::TreeNodeEx("Scene", ImGuiTreeNodeFlags_DefaultOpen)) {

        // OBJECTS FOLDER
        if (ImGui::TreeNodeEx("Objects", ImGuiTreeNodeFlags_DefaultOpen)) {
            for (auto& obj : app->scene->objects) {
                ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
                if (ImGui::TreeNodeEx((void*)obj.get(), flags, "%s", obj->name.c_str())) {
                    ImGuiTreeNodeFlags subFlags = ImGuiTreeNodeFlags_DefaultOpen;

                    // transform section
                    if (ImGui::TreeNodeEx("Transform", subFlags)) {
                        //ImGui::DragFloat3("Position", &obj->transform.position[0], 0.1f);
                        ImGui::Text("Position");
                        ImGui::SetNextItemWidth(140.0f);
                        ImGui::DragFloat3("##Pos", &obj->transform.position[0], 0.1f);
                        ImGui::TreePop();
                    }

                    // shader section
                    if (obj->shader && ImGui::TreeNodeEx("Shader", subFlags)) {
                        ImGui::Text("ID: %u", obj->shader->ID);
                        ImGui::TreePop();
                    }

                    // meshes section
                    if (!obj->meshes.empty() && ImGui::TreeNodeEx("Meshes", subFlags)) {
                        for (int i = 0; i < (int)obj->meshes.size(); i++) {
                            ImGui::BulletText("Mesh %d", i);
                        }
                        ImGui::TreePop();
                    }

                    if (ImGui::TreeNode("DANGER ZONE :)")) {
                        ImGui::Text("Metalness");
                        ImGui::SetNextItemWidth(140.0f);
                        ImGui::SliderFloat("##MetalnessFac", &obj->metalnessFac, 0.0f, 1.0f);

                        ImGui::Text("Roughness");
                        ImGui::SetNextItemWidth(140.0f);
                        ImGui::SliderFloat("##RougnessFac", &obj->roughnessFac, 0.0f, 1.0f);

                        ImGui::TreePop();
                    }

                    ImGui::TreePop(); // close object instance
                }
            }
            ImGui::TreePop(); // close objects instance
        }

        // LIGHTS FOLDER
        if (ImGui::TreeNodeEx("Lights", ImGuiTreeNodeFlags_DefaultOpen)) {
            for (int i = 0; i < (int)app->scene->lights.size(); i++) {
                auto& light = app->scene->lights[i];

                // make a unique ID for the light node
                // should probably implement a proper ID system for the engine down the line
                char lightLabel[64];
                auto dirLight = std::dynamic_pointer_cast<DirectionalLight>(light);
                bool isDir = (dirLight != nullptr);
                sprintf(lightLabel, "%s ##%p", isDir ? "Dir Light" : "Point Light", (void*)light.get());

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
            ImGui::TreePop(); // close lights folder
        }

        ImGui::TreePop(); // close root scene
    }
    ImGui::End();

    // base camera debug information,
    // standard stuff we just put here
    ImGui::Begin("Debug", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
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

    
    // PROFILER
    ImGui::Begin("Marcel's Magic Profiler", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

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
        //ImVec2(200, 80));

    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Max: %.1fms", maxFrameTime);
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

    // IRRADIANCE SKYBOX LIGHTING
    ImGui::Begin("Irradiance", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    if (app->scene->skybox && !app->scene->skybox->shCoefficients.empty()) {
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.4f, 1.0f), "SH Coefficients");
        ImGui::Separator();

        auto& sh = app->scene->skybox->shCoefficients;

        if (ImGui::BeginTable("SH_Grid", 3, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoSavedSettings)) {

            for (int i = 0; i < 9; i++) {
                ImGui::TableNextColumn();

                ImGui::PushID(i); 
                //ImGui::ColorEdit3("##col", &sh[i][0], ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
                ImGui::ColorButton("##col",
                    ImVec4(sh[i][0], sh[i][1], sh[i][2], 1.0f),
                    ImGuiColorEditFlags_NoTooltip,
                    ImVec2(20.0f, 20.0f));

                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Index: %d", i);
                }
                ImGui::PopID();
            }

            ImGui::EndTable();
        }
    }
    else {
        ImGui::TextDisabled("No SH data available for current skybox.");
    }

    ImGui::End();
}