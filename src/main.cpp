#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <ImGuizmo.h>

#include <iostream>
#include <vector>

#include "Camera.h"
#include "SceneObject.h"
#include "panels/DetailsPanel.h"
#include "panels/OutlinerPanel.h"
#include "panels/ViewportPanel.h"

namespace {
constexpr int kWindowWidth = 1600;
constexpr int kWindowHeight = 900;

static double gScrollDelta = 0.0;

void ScrollCallback(GLFWwindow* /*window*/, double /*xoffset*/, double yoffset) {
    gScrollDelta += yoffset;
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

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    OrbitCamera camera;
    camera.SetTarget(glm::vec3(0.0f, 0.0f, 0.0f), 6.0f, 45.0f, -25.0f);

    std::vector<SceneObject> sceneObjects(1);
    sceneObjects[0].name = "Cube";
    sceneObjects[0].transform.position = glm::vec3(0.0f, 0.0f, 0.0f);
    sceneObjects[0].transform.rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    sceneObjects[0].transform.scale = glm::vec3(1.0f, 1.0f, 1.0f);
    sceneObjects[0].color = glm::vec4(0.78f, 0.63f, 0.41f, 1.0f);

    int selectedIndex = 0;
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
        ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
        ImGui::End();
        ImGui::PopStyleVar(2);

        ImGui::Begin("Toolbar");
        if (ImGui::Button("Translate")) {
            gizmoOperation = ImGuizmo::TRANSLATE;
        }
        ImGui::SameLine();
        if (ImGui::Button("Rotate")) {
            gizmoOperation = ImGuizmo::ROTATE;
        }
        ImGui::SameLine();
        if (ImGui::Button("Scale")) {
            gizmoOperation = ImGuizmo::SCALE;
        }
        ImGui::SameLine();
        if (ImGui::Button(localWorld ? "Local" : "World")) {
            localWorld = !localWorld;
        }
        ImGui::End();

        if (ImGui::IsKeyPressed(ImGuiKey_W)) {
            gizmoOperation = ImGuizmo::TRANSLATE;
        }
        if (ImGui::IsKeyPressed(ImGuiKey_E)) {
            gizmoOperation = ImGuizmo::ROTATE;
        }
        if (ImGui::IsKeyPressed(ImGuiKey_R)) {
            gizmoOperation = ImGuizmo::SCALE;
        }
        if (ImGui::IsKeyPressed(ImGuiKey_L)) {
            localWorld = !localWorld;
        }

        outlinerPanel.Render(sceneObjects, &selectedIndex);
        detailsPanel.Render(sceneObjects[selectedIndex]);

        bool usedGizmo = false;
        bool hoveredByGizmo = false;
        bool viewportChange = viewportPanel.Render(sceneObjects[selectedIndex], camera, gizmoOperation,
                                                  localWorld ? ImGuizmo::LOCAL : ImGuizmo::WORLD,
                                                  localWorld, &usedGizmo, &hoveredByGizmo);
        (void)viewportChange;

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

    viewportPanel.Shutdown();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
