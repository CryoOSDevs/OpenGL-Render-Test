#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <ImGuizmo.h>

#include <algorithm>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <tiny_gltf.h>

#include "Camera.h"
#include "SceneObject.h"
#include "panels/DetailsPanel.h"
#include "panels/OutlinerPanel.h"
#include "panels/ViewportPanel.h"

namespace {
constexpr int kWindowWidth = 1600;
constexpr int kWindowHeight = 900;

static double gScrollDelta = 0.0;

struct AssetEntry {
    std::filesystem::path path;
    std::string label;
};

void ScrollCallback(GLFWwindow* /*window*/, double /*xoffset*/, double yoffset) {
    gScrollDelta += yoffset;
}

void ApplyEditorStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowPadding = ImVec2(12.0f, 10.0f);
    style.FramePadding = ImVec2(8.0f, 6.0f);
    style.ItemSpacing = ImVec2(8.0f, 8.0f);
    style.ItemInnerSpacing = ImVec2(6.0f, 6.0f);
    style.ScrollbarSize = 12.0f;
    style.GrabMinSize = 10.0f;
    style.WindowRounding = 10.0f;
    style.FrameRounding = 7.0f;
    style.ChildRounding = 10.0f;
    style.PopupRounding = 8.0f;
    style.ScrollbarRounding = 9.0f;
    style.GrabRounding = 6.0f;
    style.TabRounding = 8.0f;

    ImVec4 bg = ImVec4(0.12f, 0.13f, 0.18f, 1.0f);
    ImVec4 panel = ImVec4(0.17f, 0.18f, 0.23f, 1.0f);
    ImVec4 panelAlt = ImVec4(0.22f, 0.23f, 0.28f, 1.0f);
    ImVec4 accent = ImVec4(0.34f, 0.63f, 0.87f, 1.0f);
    ImVec4 accentSoft = ImVec4(0.19f, 0.34f, 0.54f, 1.0f);
    ImVec4 text = ImVec4(0.91f, 0.94f, 0.96f, 1.0f);
    ImVec4 dim = ImVec4(0.68f, 0.72f, 0.8f, 1.0f);

    style.Colors[ImGuiCol_WindowBg] = bg;
    style.Colors[ImGuiCol_ChildBg] = panel;
    style.Colors[ImGuiCol_PopupBg] = panelAlt;
    style.Colors[ImGuiCol_FrameBg] = panelAlt;
    style.Colors[ImGuiCol_FrameBgHovered] = accentSoft;
    style.Colors[ImGuiCol_FrameBgActive] = accentSoft;
    style.Colors[ImGuiCol_TitleBg] = bg;
    style.Colors[ImGuiCol_TitleBgActive] = bg;
    style.Colors[ImGuiCol_TitleBgCollapsed] = bg;
    style.Colors[ImGuiCol_MenuBarBg] = panel;
    style.Colors[ImGuiCol_ScrollbarBg] = panel;
    style.Colors[ImGuiCol_ScrollbarGrab] = accentSoft;
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = accent;
    style.Colors[ImGuiCol_ScrollbarGrabActive] = accent;
    style.Colors[ImGuiCol_CheckMark] = accent;
    style.Colors[ImGuiCol_SliderGrab] = accent;
    style.Colors[ImGuiCol_SliderGrabActive] = accent;
    style.Colors[ImGuiCol_Button] = accentSoft;
    style.Colors[ImGuiCol_ButtonHovered] = accent;
    style.Colors[ImGuiCol_ButtonActive] = accent;
    style.Colors[ImGuiCol_Header] = accentSoft;
    style.Colors[ImGuiCol_HeaderHovered] = accent;
    style.Colors[ImGuiCol_HeaderActive] = accent;
    style.Colors[ImGuiCol_Separator] = accentSoft;
    style.Colors[ImGuiCol_SeparatorHovered] = accent;
    style.Colors[ImGuiCol_SeparatorActive] = accent;
    style.Colors[ImGuiCol_Text] = text;
    style.Colors[ImGuiCol_TextDisabled] = dim;
    style.Colors[ImGuiCol_ResizeGrip] = accentSoft;
    style.Colors[ImGuiCol_ResizeGripHovered] = accent;
    style.Colors[ImGuiCol_ResizeGripActive] = accent;
    style.Colors[ImGuiCol_DockingEmptyBg] = bg;
    style.Colors[ImGuiCol_Tab] = panelAlt;
    style.Colors[ImGuiCol_TabHovered] = accentSoft;
    style.Colors[ImGuiCol_TabActive] = accent;
    style.Colors[ImGuiCol_TabUnfocused] = panelAlt;
    style.Colors[ImGuiCol_TabUnfocusedActive] = accentSoft;
}

SceneObject MakeDefaultCube(const std::string& name, const glm::vec3& pos = glm::vec3(0.0f, 0.0f, 0.0f)) {
    SceneObject obj;
    obj.name = name;
    obj.SetCubeMesh();
    obj.transform.position = pos;
    obj.color = glm::vec4(0.78f, 0.63f, 0.41f, 1.0f);
    return obj;
}

std::string ReadLineTrimmed(std::istream& in) {
    std::string line;
    std::getline(in, line);
    const auto first = line.find_first_not_of(" \t\r");
    if (first == std::string::npos) {
        return "";
    }
    return line.substr(first);
}

std::vector<std::string> Split(const std::string& value, char delim) {
    std::vector<std::string> tokens;
    std::stringstream stream(value);
    std::string item;
    while (std::getline(stream, item, delim)) {
        tokens.push_back(item);
    }
    return tokens;
}

bool LoadOBJFile(const std::filesystem::path& path, SceneObject& out) {
    std::ifstream stream(path);
    if (!stream.is_open()) {
        return false;
    }

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<MeshVertex> vertices;
    std::vector<unsigned int> indices;

    std::string line;
    while (std::getline(stream, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }

        if (line.rfind("v ", 0) == 0) {
            std::istringstream iss(line.substr(2));
            float x = 0.0f, y = 0.0f, z = 0.0f;
            iss >> x >> y >> z;
            positions.push_back(glm::vec3(x, y, z));
            continue;
        }

        if (line.rfind("vn ", 0) == 0) {
            std::istringstream iss(line.substr(3));
            float x = 0.0f, y = 0.0f, z = 0.0f;
            iss >> x >> y >> z;
            normals.push_back(glm::vec3(x, y, z));
            continue;
        }

        if (line.rfind("f ", 0) == 0) {
            std::vector<std::string> tokens = Split(line.substr(2), ' ');
            std::vector<std::vector<int>> faceVertices;
            bool hasNormals = false;
            for (const auto& token : tokens) {
                if (token.empty()) {
                    continue;
                }
                std::vector<int> entry;
                std::stringstream tokenStream(token);
                std::string part;
                while (std::getline(tokenStream, part, '/')) {
                    if (!part.empty()) {
                        entry.push_back(std::stoi(part));
                    }
                }
                if (entry.size() >= 1) {
                    faceVertices.push_back(entry);
                }
                if (entry.size() >= 3) {
                    hasNormals = true;
                }
            }

            if (faceVertices.size() < 3) {
                continue;
            }

            for (size_t i = 1; i + 1 < faceVertices.size(); ++i) {
                const std::vector<int>& a = faceVertices[0];
                const std::vector<int>& b = faceVertices[i];
                const std::vector<int>& c = faceVertices[i + 1];
                if (a.size() < 1 || b.size() < 1 || c.size() < 1) {
                    continue;
                }

                glm::vec3 pa = positions[a[0] - 1];
                glm::vec3 pb = positions[b[0] - 1];
                glm::vec3 pc = positions[c[0] - 1];
                glm::vec3 normal = glm::normalize(glm::cross(pb - pa, pc - pa));

                for (const auto& face : {a, b, c}) {
                    MeshVertex vertex{};
                    vertex.position = positions[face[0] - 1];
                    if (hasNormals && face.size() >= 3 && !normals.empty()) {
                        const int normalIndex = face[2] - 1;
                        if (normalIndex >= 0 && normalIndex < static_cast<int>(normals.size())) {
                            vertex.normal = normals[normalIndex];
                        }
                    }
                    if (vertex.normal == glm::vec3(0.0f)) {
                        vertex.normal = normal;
                    }
                    vertices.push_back(vertex);
                    indices.push_back(static_cast<unsigned int>(indices.size()));
                }
            }
        }
    }

    if (vertices.empty()) {
        return false;
    }

    out.SetMesh(vertices, indices);
    out.name = path.stem().string();
    return true;
}

std::vector<float> ReadFloatAccessor(const tinygltf::Model& model, const tinygltf::Accessor& accessor) {
    std::vector<float> result;
    if (accessor.bufferView < 0 || accessor.bufferView >= static_cast<int>(model.bufferViews.size())) {
        return result;
    }

    const auto& view = model.bufferViews[accessor.bufferView];
    const auto& buffer = model.buffers[view.buffer];
    const unsigned char* data = buffer.data.data() + view.byteOffset + accessor.byteOffset;
    const size_t components = tinygltf::GetNumComponentsInType(accessor.type);
    const size_t stride = view.byteStride == 0 ? (components * sizeof(float)) : view.byteStride;

    result.resize(accessor.count * components);
    for (size_t i = 0; i < accessor.count; ++i) {
        const unsigned char* offset = data + i * stride;
        std::memcpy(result.data() + i * components, offset, components * sizeof(float));
    }
    return result;
}

std::vector<unsigned int> ReadIndexAccessor(const tinygltf::Model& model, const tinygltf::Accessor& accessor) {
    std::vector<unsigned int> result;
    if (accessor.bufferView < 0 || accessor.bufferView >= static_cast<int>(model.bufferViews.size())) {
        return result;
    }

    const auto& view = model.bufferViews[accessor.bufferView];
    const auto& buffer = model.buffers[view.buffer];
    const unsigned char* data = buffer.data.data() + view.byteOffset + accessor.byteOffset;
    result.resize(accessor.count);

    if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
        for (size_t i = 0; i < accessor.count; ++i) {
            result[i] = *reinterpret_cast<const unsigned int*>(data + i * sizeof(unsigned int));
        }
    } else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
        for (size_t i = 0; i < accessor.count; ++i) {
            result[i] = *reinterpret_cast<const unsigned short*>(data + i * sizeof(unsigned short));
        }
    }
    return result;
}

bool LoadGLBFile(const std::filesystem::path& path, SceneObject& out) {
    tinygltf::TinyGLTF loader;
    tinygltf::Model model;
    std::string errors;
    std::string warnings;
    if (!loader.LoadBinaryFromFile(&model, &errors, &warnings, path.string())) {
        std::cerr << "Failed to load model: " << path << "\n" << errors << std::endl;
        return false;
    }

    if (model.meshes.empty() || model.meshes.front().primitives.empty()) {
        return false;
    }

    const tinygltf::Primitive& primitive = model.meshes.front().primitives.front();
    const auto positionIt = primitive.attributes.find("POSITION");
    if (positionIt == primitive.attributes.end()) {
        return false;
    }

    const auto& positionAccessor = model.accessors[positionIt->second];
    const auto& positionData = ReadFloatAccessor(model, positionAccessor);
    std::vector<glm::vec3> positions;
    positions.reserve(positionAccessor.count);
    for (size_t i = 0; i < positionAccessor.count; ++i) {
        const size_t base = i * 3;
        positions.push_back(glm::vec3(positionData[base], positionData[base + 1], positionData[base + 2]));
    }

    std::vector<glm::vec3> normals;
    const auto normalIt = primitive.attributes.find("NORMAL");
    if (normalIt != primitive.attributes.end()) {
        const auto& normalAccessor = model.accessors[normalIt->second];
        const auto& normalData = ReadFloatAccessor(model, normalAccessor);
        normals.reserve(normalAccessor.count);
        for (size_t i = 0; i < normalAccessor.count; ++i) {
            const size_t base = i * 3;
            normals.push_back(glm::normalize(glm::vec3(normalData[base], normalData[base + 1], normalData[base + 2])));
        }
    }

    std::vector<MeshVertex> vertices;
    vertices.reserve(positionAccessor.count);
    for (size_t i = 0; i < positions.size(); ++i) {
        MeshVertex vertex{};
        vertex.position = positions[i];
        vertex.normal = normals.empty() ? glm::vec3(0.0f, 1.0f, 0.0f) : normals[i];
        vertices.push_back(vertex);
    }

    std::vector<unsigned int> indices;
    if (primitive.indices >= 0) {
        const auto& indexAccessor = model.accessors[primitive.indices];
        indices = ReadIndexAccessor(model, indexAccessor);
    }

    if (indices.empty()) {
        indices.resize(vertices.size());
        for (size_t i = 0; i < vertices.size(); ++i) {
            indices[i] = static_cast<unsigned int>(i);
        }
    }

    out.SetMesh(vertices, indices);
    out.name = path.stem().string();
    return true;
}

bool ImportAsset(const std::filesystem::path& path, SceneObject& object) {
    const std::string ext = path.extension().string();
    if (ext == ".obj") {
        return LoadOBJFile(path, object);
    }
    if (ext == ".glb" || ext == ".gltf") {
        return LoadGLBFile(path, object);
    }
    return false;
}

std::vector<AssetEntry> RefreshAssets() {
    std::vector<AssetEntry> entries;
    const std::filesystem::path assetDir = std::filesystem::current_path() / "assets";
    if (!std::filesystem::exists(assetDir)) {
        return entries;
    }

    for (const auto& entry : std::filesystem::recursive_directory_iterator(assetDir)) {
        if (entry.is_regular_file()) {
            const std::string ext = entry.path().extension().string();
            if (ext == ".obj" || ext == ".glb" || ext == ".gltf") {
                entries.push_back({entry.path(), entry.path().filename().string()});
            }
        }
    }
    std::sort(entries.begin(), entries.end(), [](const AssetEntry& a, const AssetEntry& b) {
        return a.path.string() < b.path.string();
    });
    return entries;
}
}  // namespace

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW." << std::endl;
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);

    GLFWwindow* window = glfwCreateWindow(kWindowWidth, kWindowHeight, "Laura Editor", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window." << std::endl;
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSetScrollCallback(window, ScrollCallback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD." << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.IniFilename = nullptr;

    ApplyEditorStyle();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    OrbitCamera camera;
    camera.SetTarget(glm::vec3(0.0f, 0.0f, 0.0f), 7.5f, 45.0f, -24.0f);

    std::vector<SceneObject> sceneObjects;
    sceneObjects.push_back(MakeDefaultCube("Cube"));
    sceneObjects.back().transform.position = glm::vec3(0.0f, 0.2f, 0.0f);

    std::vector<SceneLight> lights;
    lights.push_back({glm::vec3(2.5f, 4.0f, 4.0f), glm::vec3(1.0f, 0.96f, 0.9f), 1.2f});

    std::vector<AssetEntry> assetEntries = RefreshAssets();
    std::filesystem::path selectedAsset;
    int selectedIndex = 0;
    int selectedAssetIndex = -1;
    ImGuizmo::OPERATION gizmoOperation = ImGuizmo::TRANSLATE;
    bool localWorld = false;

    ViewportPanel viewportPanel;
    DetailsPanel detailsPanel;
    OutlinerPanel outlinerPanel;

    double lastMouseX = 0.0;
    double lastMouseY = 0.0;
    bool hasMousePosition = false;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        int width = 0;
        int height = 0;
        glfwGetFramebufferSize(window, &width, &height);
        if (width > 0 && height > 0) {
            glViewport(0, 0, width, height);
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGuizmo::BeginFrame();

        ImGuiViewport* mainViewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(mainViewport->Pos);
        ImGui::SetNextWindowSize(mainViewport->Size);
        ImGui::SetNextWindowViewport(mainViewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::Begin("DockSpaceWindow", nullptr,
                     ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
                         ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
                         ImGuiWindowFlags_NoNavFocus);
        const ImGuiID dockspaceId = ImGui::GetID("LauraEditorDockspace");
        ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_AutoHideTabBar);
        ImGui::End();
        ImGui::PopStyleVar(2);

        ImGui::Begin("Toolbar", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        if (ImGui::Button("Add Cube")) {
            SceneObject newObject = MakeDefaultCube("Cube", glm::vec3(2.0f * static_cast<float>(sceneObjects.size()), 0.0f, 0.0f));
            sceneObjects.push_back(newObject);
            selectedIndex = static_cast<int>(sceneObjects.size()) - 1;
        }
        ImGui::SameLine();
        if (ImGui::Button("Add Light")) {
            lights.push_back({glm::vec3(0.0f, 2.5f, 0.0f), glm::vec3(1.0f, 0.95f, 0.84f), 1.0f});
        }
        ImGui::SameLine();
        if (ImGui::Button("Translate")) gizmoOperation = ImGuizmo::TRANSLATE;
        ImGui::SameLine();
        if (ImGui::Button("Rotate")) gizmoOperation = ImGuizmo::ROTATE;
        ImGui::SameLine();
        if (ImGui::Button("Scale")) gizmoOperation = ImGuizmo::SCALE;
        ImGui::SameLine();
        if (ImGui::Button(localWorld ? "Local" : "World")) localWorld = !localWorld;
        ImGui::SameLine();
        ImGui::Text("  %zu objects", sceneObjects.size());
        ImGui::End();

        if (ImGui::IsKeyPressed(ImGuiKey_W)) gizmoOperation = ImGuizmo::TRANSLATE;
        if (ImGui::IsKeyPressed(ImGuiKey_E)) gizmoOperation = ImGuizmo::ROTATE;
        if (ImGui::IsKeyPressed(ImGuiKey_R)) gizmoOperation = ImGuizmo::SCALE;
        if (ImGui::IsKeyPressed(ImGuiKey_L)) localWorld = !localWorld;

        ImGui::Begin("Outliner");
        for (size_t i = 0; i < sceneObjects.size(); ++i) {
            const bool selected = static_cast<int>(i) == selectedIndex;
            if (ImGui::Selectable(sceneObjects[i].name.c_str(), selected)) {
                selectedIndex = static_cast<int>(i);
            }
        }
        ImGui::End();

        ImGui::Begin("Asset Browser");
        if (ImGui::Button("Refresh")) {
            assetEntries = RefreshAssets();
        }
        for (int i = 0; i < static_cast<int>(assetEntries.size()); ++i) {
            if (ImGui::Selectable(assetEntries[i].label.c_str(), selectedAssetIndex == i)) {
                selectedAssetIndex = i;
                selectedAsset = assetEntries[i].path;
            }
        }
        if (!selectedAsset.empty()) {
            if (ImGui::Button("Import Selected")) {
                SceneObject imported;
                if (ImportAsset(selectedAsset, imported)) {
                    imported.transform.position = glm::vec3(0.0f, 0.5f, 0.0f);
                    imported.color = glm::vec4(0.59f, 0.78f, 0.85f, 1.0f);
                    sceneObjects.push_back(imported);
                    selectedIndex = static_cast<int>(sceneObjects.size()) - 1;
                }
            }
            ImGui::SameLine();
            ImGui::TextUnformatted(selectedAsset.filename().string().c_str());
        }
        ImGui::End();

        if (selectedIndex >= 0 && selectedIndex < static_cast<int>(sceneObjects.size())) {
            detailsPanel.Render(sceneObjects[selectedIndex]);
        }

        bool usedGizmo = false;
        bool hoveredByGizmo = false;
        viewportPanel.Render(sceneObjects, selectedIndex, camera, lights, gizmoOperation,
                            localWorld ? ImGuizmo::LOCAL : ImGuizmo::WORLD,
                            localWorld, &usedGizmo, &hoveredByGizmo);

        if (viewportPanel.IsHovered() && !usedGizmo && !hoveredByGizmo) {
            double mouseX = 0.0;
            double mouseY = 0.0;
            glfwGetCursorPos(window, &mouseX, &mouseY);

            if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
                if (hasMousePosition) {
                    const float deltaX = static_cast<float>(mouseX - lastMouseX);
                    const float deltaY = static_cast<float>(mouseY - lastMouseY);
                    camera.Orbit(deltaX, deltaY);
                }
                lastMouseX = mouseX;
                lastMouseY = mouseY;
                hasMousePosition = true;
            } else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS) {
                if (hasMousePosition) {
                    const float deltaX = static_cast<float>(mouseX - lastMouseX);
                    const float deltaY = static_cast<float>(mouseY - lastMouseY);
                    camera.Pan(deltaX, deltaY);
                }
                lastMouseX = mouseX;
                lastMouseY = mouseY;
                hasMousePosition = true;
            } else {
                hasMousePosition = false;
            }
        }

        if (gScrollDelta != 0.0) {
            camera.Zoom(static_cast<float>(gScrollDelta));
            gScrollDelta = 0.0;
        }

        ImGui::Render();
        glClearColor(0.04f, 0.05f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            GLFWwindow* backupCurrentContext = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backupCurrentContext);
        }

        glfwSwapBuffers(window);
    }

    sceneObjects.clear();
    viewportPanel.Shutdown();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
