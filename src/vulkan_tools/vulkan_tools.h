#ifndef VULKAN_TOOLS_H
#define VULKAN_TOOLS_H
#pragma once
#include <iostream>
#include <vector>
#include <vulkan/vulkan.h>

// forward delcare
std::ostream &operator<<(std::ostream &os,
                         const VkPhysicalDeviceProperties &props);

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

VkInstance create_vulkan_instance();

VkPhysicalDevice get_vulkan_physical_device(VkInstance instance);

VkDevice get_vulkan_device(VkInstance instance,
                           VkPhysicalDevice physicalDevice);

VkImage create_image(VkDevice device, uint32_t width, uint32_t height);
void allocate_image(VkDevice device, VkImage image);

VkImageView create_image_view(VkDevice device, VkImage image);
VkRenderPass create_render_pass(VkDevice device);
VkFramebuffer create_framebuffer(VkDevice device, VkRenderPass renderPass,
                                 VkImageView imageView, uint32_t width,
                                 uint32_t height);
VkBuffer create_vertex_buffer(VkDevice device,
                              const std::vector<Vertex> &vertices,
                              std::vector<uint16_t> indices);

} // namespace VK_TOOLS

#endif