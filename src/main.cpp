#include <assert.h>
#include <iostream>
#include <string>
#include <vector>

#define VK_USE_PLATFORM_WIN32_KHR
#include "vulkan_tools/vulkan_tools.h"

#include <glad/glad.h>
#include <glfw/glfw3.h>

#define IMGUI_IMPL_OPENGL_LOADER_GLAD
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "imgui.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

static GLuint createGLTextureFromVulkanImage(HANDLE win32Handle, int width, int height) {
  uint32_t bpp = 4;
  GLuint texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

  glBindTexture(GL_TEXTURE_2D, 0);

  // Import the Vulkan image into OpenGL
  glImportMemoryWin32HandleEXT(texture, (GLuint64)width * height * bpp * 2, GL_HANDLE_TYPE_OPAQUE_WIN32_EXT, win32Handle);

  return texture;
}

static void ImGuiInit(GLFWwindow *window) {
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
static void ImGuiBeginFrame() {
  // Start the Dear ImGui frame
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  ImGui::DockSpaceOverViewport(NULL, NULL, ImGuiDockNodeFlags_None | ImGuiDockNodeFlags_PassthruCentralNode /*|ImGuiDockNodeFlags_NoResize*/);
}
static void ImGuiEndFrame() {

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

static void display_extensions(vk::PhysicalDevice &physicalDevice, vk::Instance &instance) {
  std::vector<std::string> extensions = get_physical_device_available_extensions(physicalDevice);
  ImGui::Begin("Extensions");
  static char filter[256] = {0};
  if (ImGui::InputText("Filter", filter, 256)) {
  }

  ImGui::PushItemWidth(-1);
  ImGui::BeginListBox("##Extensions", ImVec2(0, -1));
  static int selected = -1;
  for (size_t i = 0; i < extensions.size(); i++) {
    const auto &extension = extensions[i];
    if (std::strstr(extension.c_str(), filter)) {
      bool isSelected = selected == i;
      if (ImGui::Selectable(extension.c_str(), isSelected)) {
        selected = i;
      }
      if (isSelected) {
        ImGui::SetItemDefaultFocus();
      }
    }
  }

  ImGui::EndListBox();
  ImGui::PopItemWidth();
  ImGui::End();
}

void fillVulkanImage(vk::Device device, vk::DeviceMemory memory, int width, int height) {
  void *data;
  vk::Result result = device.mapMemory(memory, 0, VK_WHOLE_SIZE, {}, &data);
  assert(result == vk::Result::eSuccess);

  uint32_t *pixels = static_cast<uint32_t *>(data);
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      pixels[y * width + x] = 0xFF0000FF; // Fill with blue color
    }
  }

  device.unmapMemory(memory);
}

int main(int argc, char **argv) {

  vk::Instance vk_instance = create_vulkan_instance();
  vk::DispatchLoaderDynamic dldi(vk_instance, vkGetInstanceProcAddr);

  vk::PhysicalDevice physical_device = get_vulkan_physical_device(vk_instance);

  bool has_ext = check_physical_device_extension_support(physical_device, {"VK_KHR_external_memory_win32"});
  std::cout << "has VK_KHR_external_memory_win32 : " << (has_ext ? "true" : "false") << std::endl;

  vk::Device device = get_vulkan_device(vk_instance, physical_device);
  std::cout << "Device OK: " << device << std::endl;

  // init Dynamic loader ?
  dldi.init(device);

  vk::Image image = create_image(device, 128, 128);
  std::cout << "Image OK: " << image << std::endl;

  vk::DeviceMemory vulkanMemory = bind_image_to_device_memory(device, physical_device, image);

  vk::MemoryGetWin32HandleInfoKHR getHandleInfo = {};

  getHandleInfo.memory = vulkanMemory;
  getHandleInfo.handleType = vk::ExternalMemoryHandleTypeFlagBits::eOpaqueWin32;

  // use dynamic loader here
  HANDLE win32Handle = device.getMemoryWin32HandleKHR(getHandleInfo, dldi);
  std::cout << "Win32 Handle OK: " << win32Handle << std::endl;

  vk::ImageView imageView = create_image_view(device, image);
  std::cout << "ImageView OK: " << imageView << std::endl;

  vk::RenderPass renderPass = create_render_pass(device);
  std::cout << "RenderPass OK: " << renderPass << std::endl;
  vk::Framebuffer framebuffer = create_framebuffer(device, renderPass, imageView, 128, 128);
  std::cout << "Framebuffer OK: " << framebuffer << std::endl;
  create_graphics_pipeline(device);

  std::vector<Vertex> vertices = {
      {{0.0f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}}, // Red vertex
      {{0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},  // Green vertex
      {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}}  // Blue vertex
  };

  std::vector<uint16_t> indices = {0, 1, 2};

  vk::Buffer vBuffer = create_vertex_buffer(device, vertices, indices);
  std::cout << "Vertex Buffer OK: " << vBuffer << std::endl;

  if (!glfwInit()) {
    printf("problem with GLFW\n");
    return -1;
  }

  GLFWwindow *window = glfwCreateWindow(1280, 720, "Starter Project", NULL, NULL);

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

  // Fill the Vulkan image with some test data
  fillVulkanImage(device, vulkanMemory, 128, 128);
  GLuint texture = createGLTextureFromVulkanImage(win32Handle, 128, 128);

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

    display_extensions(physical_device, vk_instance);

    // Render the Vulkan image as an OpenGL texture in ImGui
    ImGui::Begin("Vulkan Image");
    ImGui::Image((ImTextureID)(void *)(intptr_t)texture, ImVec2(128, 128));
    ImGui::End();

    ImGuiEndFrame();

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();

  printf("GoodBye... \n");
  return 0;
}