#ifndef VULKAN_TOOLS_H
#define VULKAN_TOOLS_H
#pragma once
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>
// #include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>
// forward delcare
std::ostream &operator<<(std::ostream &os,
                         const vk::PhysicalDeviceProperties &props);

namespace VK_TOOLS {
bool checkValidationLayerSupport();
struct Vertex {
  float position[3];
  float color[3];
};

const std::vector<const char *> validationLayers = {
    "VK_LAYER_KHRONOS_validation"};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

vk::Instance create_vulkan_instance();

vk::PhysicalDevice get_vulkan_physical_device(vk::Instance &instance);

vk::Device get_vulkan_device(vk::Instance &instance,
                             vk::PhysicalDevice &physicalDevice);

vk::Image create_image(vk::Device &device, uint32_t width, uint32_t height);
void allocate_image(vk::Device &device, vk::Image &image);

vk::ImageView create_image_view(vk::Device &device, vk::Image &image);
vk::RenderPass create_render_pass(vk::Device &device);
VkFramebuffer create_framebuffer(VkDevice device, VkRenderPass renderPass,
                                 VkImageView imageView, uint32_t width,
                                 uint32_t height);

// graphics pipeline
std::vector<char> readFile(const std::string &filename);
VkShaderModule createShaderModule(VkDevice device,
                                  const std::vector<char> &code);
VkPipelineLayout create_graphics_pipeline(VkDevice device);

// fixed functions
struct fixedFunctions {

  VkPipelineDynamicStateCreateInfo dynamicState;
  VkPipelineVertexInputStateCreateInfo vertexInputInfo;
  VkPipelineInputAssemblyStateCreateInfo inputAssembly;
  VkViewport viewport;
  VkPipelineViewportStateCreateInfo viewportState;
  VkRect2D scissor;
  VkPipelineRasterizationStateCreateInfo rasterizer;
  VkPipelineMultisampleStateCreateInfo multisampling;
  VkPipelineColorBlendAttachmentState colorBlendAttachment;
  VkPipelineColorBlendStateCreateInfo colorBlending;
};

VkBuffer create_vertex_buffer(VkDevice device,
                              const std::vector<Vertex> &vertices,
                              std::vector<uint16_t> indices);

} // namespace VK_TOOLS

#endif
