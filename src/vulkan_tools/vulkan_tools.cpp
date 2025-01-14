#include "vulkan_tools.h"

namespace VK_TOOLS {

bool checkValidationLayerSupport() {
  uint32_t layerCount;
  vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

  std::vector<VkLayerProperties> availableLayers(layerCount);
  vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

  for (const char *layerName : validationLayers) {
    bool layerFound = false;

    for (const auto &layerProperties : availableLayers) {
      if (strcmp(layerName, layerProperties.layerName) == 0) {
        layerFound = true;
        break;
      }
    }

    if (!layerFound) {
      return false;
    }
  }

  return true;
}

VkInstance create_vulkan_instance() {
  if (enableValidationLayers && !checkValidationLayerSupport()) {
    throw std::runtime_error("validation layers requested, but not available!");
  }
  VkInstance instance;
  VkInstanceCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  if (enableValidationLayers) {
    createInfo.enabledLayerCount =
        static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();
  } else {
    createInfo.enabledLayerCount = 0;
  }
  vkCreateInstance(&createInfo, nullptr, &instance);
  std::cout << "Vulkan instance created" << std::endl;
  return instance;
}

VkPhysicalDevice get_vulkan_physical_device(VkInstance instance) {
  VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
  std::vector<VkPhysicalDevice> devices(deviceCount);
  vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
  physicalDevice = devices[0]; // Assume the first device

  std::cout
      << "Vulkan physical device created (automatically picked up first device)"
      << std::endl;
  VkPhysicalDeviceProperties props;
  vkGetPhysicalDeviceProperties(physicalDevice, &props);
  std::cout << props << std::endl;
  return physicalDevice;
}

VkDevice get_vulkan_device(VkInstance instance,
                           VkPhysicalDevice physicalDevice) {
  // Get queue family supporting graphics
  uint32_t queueFamilyIndex = 0;
  VkQueueFamilyProperties queueFamilyProps[32];
  uint32_t queueCount;
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount,
                                           nullptr);
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount,
                                           queueFamilyProps);

  for (uint32_t i = 0; i < queueCount; ++i) {
    if (queueFamilyProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      queueFamilyIndex = i;
      break;
    }
  }

  // Create logical device
  VkDevice device;
  VkDeviceQueueCreateInfo queueCreateInfo = {};
  queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
  queueCreateInfo.queueCount = 1;
  float queuePriority = 1.0f;
  queueCreateInfo.pQueuePriorities = &queuePriority;

  VkDeviceCreateInfo deviceCreateInfo = {};
  deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  deviceCreateInfo.queueCreateInfoCount = 1;
  deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;

  vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device);

  return device;
}

VkImage create_image(VkDevice device, uint32_t width, uint32_t height) {
  VkImageCreateInfo imageInfo{};
  imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo.imageType = VK_IMAGE_TYPE_2D;
  imageInfo.extent.width = width;   // Image width
  imageInfo.extent.height = height; // Image height
  imageInfo.extent.depth = 1;
  imageInfo.mipLevels = 1;
  imageInfo.arrayLayers = 1;
  imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM; // Example format
  imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
  imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageInfo.usage =
      VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
  imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  VkImage image;
  vkCreateImage(device, &imageInfo, nullptr, &image);

  return image;
}

void allocate_image(VkDevice device, VkImage image) {
  VkMemoryRequirements memRequirements;
  vkGetImageMemoryRequirements(device, image, &memRequirements);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

  VkDeviceMemory imageMemory;
  vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory);
  vkBindImageMemory(device, image, imageMemory, 0);
}

VkImageView create_image_view(VkDevice device, VkImage image) {
  VkImageViewCreateInfo viewInfo{};
  viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.image = image;
  viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
  viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  viewInfo.subresourceRange.baseMipLevel = 0;
  viewInfo.subresourceRange.levelCount = 1;
  viewInfo.subresourceRange.baseArrayLayer = 0;
  viewInfo.subresourceRange.layerCount = 1;

  VkImageView imageView;
  vkCreateImageView(device, &viewInfo, nullptr, &imageView);

  return imageView;
}

VkFramebuffer create_framebuffer(VkDevice device, VkRenderPass renderPass,
                                 VkImageView imageView, uint32_t width,
                                 uint32_t height) {
  VkFramebufferCreateInfo framebufferInfo{};
  framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  framebufferInfo.renderPass = renderPass; // Your render pass
  framebufferInfo.attachmentCount = 1;
  framebufferInfo.pAttachments = &imageView;
  framebufferInfo.width = width;
  framebufferInfo.height = height;
  framebufferInfo.layers = 1;

  VkFramebuffer framebuffer;
  vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffer);

  return framebuffer;
}

VkRenderPass create_render_pass(VkDevice device) {
  VkAttachmentDescription colorAttachment{};
  colorAttachment.format = VK_FORMAT_R8G8B8A8_UNORM;
  colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
  colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

  colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

  colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkAttachmentReference colorAttachmentRef{};
  colorAttachmentRef.attachment = 0;
  colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass{};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &colorAttachmentRef;

  VkRenderPass renderPass;
  // VkPipelineLayout pipelineLayout;
  VkRenderPassCreateInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount = 1;
  renderPassInfo.pAttachments = &colorAttachment;
  renderPassInfo.subpassCount = 1;
  renderPassInfo.pSubpasses = &subpass;

  if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create render pass!");
  }

  return renderPass;
}

std::vector<char> readFile(const std::string &filename) {
  std::ifstream file(filename, std::ios::ate | std::ios::binary);

  if (!file.is_open()) {
    throw std::runtime_error("failed to open file!");
  }

  size_t fileSize = (size_t)file.tellg();
  std::vector<char> buffer(fileSize);

  file.seekg(0);
  file.read(buffer.data(), fileSize);

  file.close();

  return buffer;
}

VkShaderModule createShaderModule(VkDevice device,
                                  const std::vector<char> &code) {
  VkShaderModuleCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = code.size();
  createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());
  VkShaderModule shaderModule;
  if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create shader module!");
  }

  return shaderModule;
}

VkPipelineLayout create_graphics_pipeline(VkDevice device) {
  auto vertShaderCode = readFile("./compiled_shaders/shader__vert.spv");
  auto fragShaderCode = readFile("./compiled_shaders/shader__frag.spv");

  VkShaderModule vertShaderModule = createShaderModule(device, vertShaderCode);
  VkShaderModule fragShaderModule = createShaderModule(device, fragShaderCode);

  VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
  vertShaderStageInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;

  vertShaderStageInfo.module = vertShaderModule;
  vertShaderStageInfo.pName = "main";

  VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
  fragShaderStageInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  fragShaderStageInfo.module = fragShaderModule;
  fragShaderStageInfo.pName = "main";

  VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo,
                                                    fragShaderStageInfo};

  VkPipelineLayout pipelineLayout;

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = 0;            // Optional
  pipelineLayoutInfo.pSetLayouts = nullptr;         // Optional
  pipelineLayoutInfo.pushConstantRangeCount = 0;    // Optional
  pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

  if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr,
                             &pipelineLayout) != VK_SUCCESS) {
    throw std::runtime_error("failed to create pipeline layout!");
  }

  // destroy shader modules
  vkDestroyShaderModule(device, fragShaderModule, nullptr);
  vkDestroyShaderModule(device, vertShaderModule, nullptr);

  return pipelineLayout;
}
VkBuffer create_vertex_buffer(VkDevice device,
                              const std::vector<Vertex> &vertices,
                              std::vector<uint16_t> indices) {
  // Create Vulkan buffers (vertex buffer, index buffer, etc.)
  VkBuffer vertexBuffer;
  VkBufferCreateInfo bufferInfo = {};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = sizeof(Vertex) * vertices.size();
  bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

  vkCreateBuffer(device, &bufferInfo, nullptr, &vertexBuffer);

  return vertexBuffer;
}
} // namespace VK_TOOLS

// iostream utils

std::ostream &operator<<(std::ostream &os,
                         const VkPhysicalDeviceProperties &props) {
  os << "---------------------------\n";
  os << "VkPhysicalDeviceProperties\n";
  os << "  device Name : " << props.deviceName << "\n";
  switch (props.deviceType) {
  case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
    os << "  device Type : VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU\n";
    break;
  case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
    os << "  device Type : VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU\n";
    break;
  default:
    os << "  device Type : Unknown\n";
    break;
  }
  return os;
}