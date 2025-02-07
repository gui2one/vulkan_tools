#include <assert.h>
#include <iostream>
#include <string>
#include <vector>

#include <glad/glad.h>
#include <glfw/glfw3.h>

#define IMGUI_IMPL_OPENGL_LOADER_GLAD
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "imgui.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

#include "vulkan_tools/vulkan_tools.h"
void ImGuiInit(GLFWwindow *window) {
  // init ImGui
  // Setup Dear ImGui context

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  // io.Fonts->AddFontFromFileTTF(ORBITONS_RES_DIR
  // "/fonts/JetBrainsMono-Regular.ttf", 16);

  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  //   io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

  ImGui_ImplGlfw_InitForOpenGL(window, true);
  const char *glsl_version = "#version 130";
  ImGui_ImplOpenGL3_Init(glsl_version);

  ////////////
  // end imgui config
  ///////////
}
void ImGuiBeginFrame() {
  // Start the Dear ImGui frame
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  ImGui::DockSpaceOverViewport(
      NULL, NULL,
      ImGuiDockNodeFlags_None |
          ImGuiDockNodeFlags_PassthruCentralNode /*|ImGuiDockNodeFlags_NoResize*/);
}
void ImGuiEndFrame() {

  // Rendering
  ImGui::Render();

  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  ImGui::EndFrame();

  ImGuiIO &io = ImGui::GetIO();

  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    GLFWwindow *backup_current_context = glfwGetCurrentContext();

    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();

    glfwMakeContextCurrent(backup_current_context);
  }
}

using namespace VK_TOOLS;
int main(int argc, char **argv) {

  std::cout << argv[0] << std::endl;
  vk::Instance vk_instance = create_vulkan_instance();
  vk::PhysicalDevice physical_device = get_vulkan_physical_device(vk_instance);

  vk::Device device = get_vulkan_device(vk_instance, physical_device);
  std::cout << "Device created : " << device << std::endl;

  VkImage image = create_image(device, 128, 128);
  allocate_image(device, image);
  std::cout << "Image created : " << image << std::endl;

  VkImageView imageView = create_image_view(device, image);
  std::cout << "ImageView created : " << imageView << std::endl;

  VkRenderPass renderPass = create_render_pass(device);

  VkFramebuffer framebuffer =
      create_framebuffer(device, renderPass, imageView, 128, 128);

  create_graphics_pipeline(device);

  std::vector<Vertex> vertices = {
      {{0.0f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}}, // Red vertex
      {{0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},  // Green vertex
      {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}}  // Blue vertex
  };

  std::vector<uint16_t> indices = {0, 1, 2};

  VkBuffer vBuffer = create_vertex_buffer(device, vertices, indices);
  std::cout << "VkBuffer created : ";
  std::cout << vBuffer << std::endl;

  if (!glfwInit()) {
    printf("problem with GLFW\n");
    return -1;
  }

  GLFWwindow *window =
      glfwCreateWindow(1280, 720, "Starter Project", NULL, NULL);

  if (window == NULL) {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(window);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return -1;
  }

  ImGuiInit(window);

  glViewport(0, 0, 640, 360);
  glfwSwapInterval(1);
  while (!glfwWindowShouldClose(window)) {
    ImGuiBeginFrame();

    glClearColor(0.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    static bool showDemoWindow = true;
    if (showDemoWindow) {
      ImGui::ShowDemoWindow(&showDemoWindow);
    }

    ImGuiEndFrame();

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();

  printf("GoodBye... \n");
  return 0;
}