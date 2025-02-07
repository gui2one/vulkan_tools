#include "vulkan_tools.h"

namespace VK_TOOLS {

bool checkValidationLayerSupport() {

  std::vector<vk::LayerProperties> availableLayers =
      vk::enumerateInstanceLayerProperties();

  for (const char *layerName : validationLayers) {
    bool layerFound = false;

    for (const auto &layerProperties : availableLayers) {
      if (strcmp(layerName, layerProperties.layerName) == 0) {
        layerFound = true;
        std::cout << "Validation Layer added : " << layerName << std::endl;
        break;
      }
    }

    if (!layerFound) {
      return false;
    }
  }

  return true;
}

vk::Instance create_vulkan_instance() {
  if (enableValidationLayers && !checkValidationLayerSupport()) {
    throw std::runtime_error("validation layers requested, but not available!");
  }

  vk::InstanceCreateInfo createInfo = {};
  if (enableValidationLayers) {
    createInfo.enabledLayerCount =
        static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();
  } else {
    createInfo.enabledLayerCount = 0;
  }
  vk::Instance instance = vk::createInstance(createInfo, nullptr);
  std::cout << "Vulkan instance created" << std::endl;
  return instance;
}

vk::PhysicalDevice get_vulkan_physical_device(vk::Instance &instance) {
  vk::PhysicalDevice physicalDevice = VK_NULL_HANDLE;
  uint32_t deviceCount = 0;

  std::vector<vk::PhysicalDevice> devices = instance.enumeratePhysicalDevices();
  physicalDevice = devices[0]; // Assume the first device

  std::cout
      << "Vulkan physical device created (automatically picked up first device)"
      << std::endl;

  vk::PhysicalDeviceProperties props = physicalDevice.getProperties();
  std::cout << props << std::endl;
  return physicalDevice;
}

vk::Device get_vulkan_device(vk::Instance &instance,
                             vk::PhysicalDevice &physicalDevice) {
  // Get queue family supporting graphics
  uint32_t queueFamilyIndex = 0;
  std::vector<vk::QueueFamilyProperties> queueFamilyProps =
      physicalDevice.getQueueFamilyProperties();

  for (uint32_t i = 0; i < queueFamilyProps.size(); ++i) {
    if (queueFamilyProps.size() > 0) {

      if (queueFamilyProps[i].queueFlags & vk::QueueFlagBits::eGraphics) {
        queueFamilyIndex = i;
        break;
      }
    }
  }

  // Create logical device
  vk::Device device;
  vk::DeviceQueueCreateInfo queueCreateInfo = {};
  queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
  queueCreateInfo.queueCount = 1;
  float queuePriority = 1.0f;
  queueCreateInfo.pQueuePriorities = &queuePriority;

  vk::DeviceCreateInfo deviceCreateInfo = {};
  deviceCreateInfo.queueCreateInfoCount = 1;
  deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;

  physicalDevice.createDevice(&deviceCreateInfo, nullptr, &device);

  return device;
}

vk::Image create_image(vk::Device &device, uint32_t width, uint32_t height) {
  vk::ImageCreateInfo imageInfo{};

  imageInfo.imageType = vk::ImageType::e2D;
  imageInfo.extent.width = width;   // Image width
  imageInfo.extent.height = height; // Image height
  imageInfo.extent.depth = 1;
  imageInfo.mipLevels = 1;
  imageInfo.arrayLayers = 1;
  imageInfo.format = vk::Format::eR8G8B8A8Unorm; // Example format
  imageInfo.tiling = vk::ImageTiling::eOptimal;  // VK_IMAGE_TILING_OPTIMAL;
  imageInfo.initialLayout =
      vk::ImageLayout::eUndefined; // VK_IMAGE_LAYOUT_UNDEFINED;
  imageInfo.usage = vk::ImageUsageFlagBits::eColorAttachment |
                    vk::ImageUsageFlagBits::eTransferSrc;
  imageInfo.samples = vk::SampleCountFlagBits::e1; // VK_SAMPLE_COUNT_1_BIT;
  imageInfo.sharingMode =
      vk::SharingMode::eExclusive; // VK_SHARING_MODE_EXCLUSIVE;

  vk::Image image = device.createImage(imageInfo);

  return image;
}

void allocate_image(vk::Device &device, vk::Image &image) {
  vk::MemoryRequirements memRequirements =
      device.getImageMemoryRequirements(image);

  vk::MemoryAllocateInfo allocInfo{};
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

  VkDeviceMemory imageMemory = device.allocateMemory(allocInfo);
  device.bindImageMemory(image, imageMemory, 0);
}

vk::ImageView create_image_view(vk::Device &device, vk::Image &image) {
  vk::ImageViewCreateInfo viewInfo{};
  viewInfo.image = image;
  viewInfo.viewType = vk::ImageViewType::e2D;   // VK_IMAGE_VIEW_TYPE_2D;
  viewInfo.format = vk::Format::eR8G8B8A8Unorm; // VK_FORMAT_R8G8B8A8_UNORM;
  viewInfo.subresourceRange.aspectMask =
      vk::ImageAspectFlagBits::eColor; // VK_IMAGE_ASPECT_COLOR_BIT;
  viewInfo.subresourceRange.baseMipLevel = 0;
  viewInfo.subresourceRange.levelCount = 1;
  viewInfo.subresourceRange.baseArrayLayer = 0;
  viewInfo.subresourceRange.layerCount = 1;

  vk::ImageView imageView = device.createImageView(viewInfo);

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

vk::RenderPass create_render_pass(vk::Device &device) {
  vk::AttachmentDescription colorAttachment{};
  colorAttachment.format =
      vk::Format::eR8G8B8A8Unorm; // VK_FORMAT_R8G8B8A8_UNORM;
  colorAttachment.samples =
      vk::SampleCountFlagBits::e1; // VK_SAMPLE_COUNT_1_BIT;
  colorAttachment.loadOp =
      vk::AttachmentLoadOp::eClear; // VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachment.storeOp =
      vk::AttachmentStoreOp::eStore; // VK_ATTACHMENT_STORE_OP_STORE;

  colorAttachment.stencilLoadOp =
      vk::AttachmentLoadOp::eDontCare; // VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachment.stencilStoreOp =
      vk::AttachmentStoreOp::eDontCare; // VK_ATTACHMENT_STORE_OP_DONT_CARE;

  colorAttachment.initialLayout =
      vk::ImageLayout::eUndefined; // VK_IMAGE_LAYOUT_UNDEFINED;
  colorAttachment.finalLayout = vk::ImageLayout::
      eColorAttachmentOptimal; // VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  vk::AttachmentReference colorAttachmentRef{};
  colorAttachmentRef.attachment = 0;
  colorAttachmentRef.layout = vk::ImageLayout::
      eColorAttachmentOptimal; // VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  vk::SubpassDescription subpass{};
  subpass.pipelineBindPoint =
      vk::PipelineBindPoint::eGraphics; // VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &colorAttachmentRef;

  vk::RenderPass renderPass;
  // VkPipelineLayout pipelineLayout;
  vk::RenderPassCreateInfo renderPassInfo{};
  // renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount = 1;
  renderPassInfo.pAttachments = &colorAttachment;
  renderPassInfo.subpassCount = 1;
  renderPassInfo.pSubpasses = &subpass;

  if (device.createRenderPass(&renderPassInfo, nullptr, &renderPass) !=
      vk::Result::eSuccess) {
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