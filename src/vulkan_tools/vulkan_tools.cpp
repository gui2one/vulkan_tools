#include "vulkan_tools.h"

namespace VK_TOOLS {

bool checkValidationLayerSupport() {

  std::vector<vk::LayerProperties> availableLayers = vk::enumerateInstanceLayerProperties();

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
    createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();
  } else {
    createInfo.enabledLayerCount = 0;
  }

  const char *instanceExtensions[] = {VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME};
  createInfo.enabledExtensionCount = 1;
  createInfo.ppEnabledExtensionNames = instanceExtensions;
  vk::Instance instance = vk::createInstance(createInfo, nullptr);
  std::cout << "Vulkan instance created" << std::endl;

  return instance;
}

vk::PhysicalDevice get_vulkan_physical_device(vk::Instance &instance) {
  vk::PhysicalDevice physicalDevice = VK_NULL_HANDLE;
  uint32_t deviceCount = 0;

  std::vector<vk::PhysicalDevice> devices = instance.enumeratePhysicalDevices();
  physicalDevice = devices[0]; // Assume the first device

  std::cout << "Vulkan physical device created (automatically picked up first device)" << std::endl;

  vk::PhysicalDeviceProperties props = physicalDevice.getProperties();
  std::cout << props << std::endl;

  // auto extensions = physicalDevice.enumerateDeviceExtensionProperties();

  // std::cout << "Available device extensions:" << std::endl;
  // for (const auto &extension : extensions) {
  //   std::cout << extension.extensionName << std::endl;
  // }
  // std::cout << "---------------------------" << std::endl;

  return physicalDevice;
}

vk::Device get_vulkan_device(vk::Instance &instance, vk::PhysicalDevice &physicalDevice) {
  // Get queue family supporting graphics
  uint32_t queueFamilyIndex = 0;
  std::vector<vk::QueueFamilyProperties> queueFamilyProps = physicalDevice.getQueueFamilyProperties();

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
  const char *deviceExtensions[] = {VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME, VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME};
  deviceCreateInfo.enabledExtensionCount = 2;
  deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions;
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
  imageInfo.format = vk::Format::eR8G8B8A8Unorm;         // Example format
  imageInfo.tiling = vk::ImageTiling::eOptimal;          // VK_IMAGE_TILING_OPTIMAL;
  imageInfo.initialLayout = vk::ImageLayout::eUndefined; // VK_IMAGE_LAYOUT_UNDEFINED;
  imageInfo.usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc;
  // imageInfo.usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst;
  imageInfo.samples = vk::SampleCountFlagBits::e1; // VK_SAMPLE_COUNT_1_BIT;
  // imageInfo.sharingMode = vk::SharingMode::eExclusive;

  // Enable external memory
  vk::ExternalMemoryImageCreateInfo externalImageInfo{};
  externalImageInfo.handleTypes = vk::ExternalMemoryHandleTypeFlagBits::eOpaqueWin32;
  imageInfo.pNext = &externalImageInfo;

  vk::Image image = device.createImage(imageInfo);

  return image;
}

void allocate_image(vk::Device &device, vk::Image &image) {
  vk::MemoryRequirements memRequirements = device.getImageMemoryRequirements(image);

  vk::MemoryAllocateInfo allocInfo{};
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

  vk::DeviceMemory imageMemory = device.allocateMemory(allocInfo);
  device.bindImageMemory(image, imageMemory, 0);
}

static uint32_t findMemoryType(vk::PhysicalDevice physicalDevice, uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
  vk::PhysicalDeviceMemoryProperties memProperties = physicalDevice.getMemoryProperties();

  for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
    if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
      return i;
    }
  }

  throw std::runtime_error("Failed to find a suitable memory type!");
}

vk::DeviceMemory bind_image_to_device_memory(vk::Device &device, vk::PhysicalDevice &physicalDevice, vk::Image &vulkanImage) {
  vk::MemoryRequirements memRequirements = device.getImageMemoryRequirements(vulkanImage);

  vk::ExportMemoryAllocateInfo exportAllocInfo{};
  exportAllocInfo.handleTypes = vk::ExternalMemoryHandleTypeFlagBits::eOpaqueWin32;

  vk::MemoryAllocateInfo allocInfo{};
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);
  allocInfo.pNext = &exportAllocInfo;

  vk::DeviceMemory vulkanMemory = device.allocateMemory(allocInfo);
  device.bindImageMemory(vulkanImage, vulkanMemory, 0);

  return vulkanMemory;
}
vk::ImageView create_image_view(vk::Device &device, vk::Image &image) {
  vk::ImageViewCreateInfo viewInfo{};
  viewInfo.image = image;
  viewInfo.viewType = vk::ImageViewType::e2D;                             // VK_IMAGE_VIEW_TYPE_2D;
  viewInfo.format = vk::Format::eR8G8B8A8Unorm;                           // VK_FORMAT_R8G8B8A8_UNORM;
  viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor; // VK_IMAGE_ASPECT_COLOR_BIT;
  viewInfo.subresourceRange.baseMipLevel = 0;
  viewInfo.subresourceRange.levelCount = 1;
  viewInfo.subresourceRange.baseArrayLayer = 0;
  viewInfo.subresourceRange.layerCount = 1;

  vk::ImageView imageView = device.createImageView(viewInfo);

  return imageView;
}

vk::Framebuffer create_framebuffer(vk::Device &device, vk::RenderPass &renderPass, vk::ImageView &imageView, uint32_t width, uint32_t height) {
  vk::FramebufferCreateInfo framebufferInfo{};
  framebufferInfo.renderPass = renderPass; // Your render pass
  framebufferInfo.attachmentCount = 1;
  framebufferInfo.pAttachments = &imageView;
  framebufferInfo.width = width;
  framebufferInfo.height = height;
  framebufferInfo.layers = 1;

  vk::Framebuffer framebuffer = device.createFramebuffer(framebufferInfo);

  return framebuffer;
}

vk::RenderPass create_render_pass(vk::Device &device) {
  vk::AttachmentDescription colorAttachment{};
  colorAttachment.format = vk::Format::eR8G8B8A8Unorm;
  colorAttachment.samples = vk::SampleCountFlagBits::e1;
  colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
  colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;

  colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
  colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;

  colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
  colorAttachment.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;

  vk::AttachmentReference colorAttachmentRef{};
  colorAttachmentRef.attachment = 0;
  colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

  vk::SubpassDescription subpass{};
  subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &colorAttachmentRef;

  vk::RenderPass renderPass;
  vk::RenderPassCreateInfo renderPassInfo{};
  renderPassInfo.attachmentCount = 1;
  renderPassInfo.pAttachments = &colorAttachment;
  renderPassInfo.subpassCount = 1;
  renderPassInfo.pSubpasses = &subpass;

  if (device.createRenderPass(&renderPassInfo, nullptr, &renderPass) != vk::Result::eSuccess) {
    throw std::runtime_error("failed to create render pass!");
  }

  return renderPass;
}

std::vector<char> read_file(const std::string &filename) {
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

vk::ShaderModule create_shader_module(vk::Device &device, const std::vector<char> &code) {
  vk::ShaderModuleCreateInfo createInfo{};
  createInfo.codeSize = code.size();
  createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());
  vk::ShaderModule shaderModule;
  if (device.createShaderModule(&createInfo, nullptr, &shaderModule) != vk::Result::eSuccess) {
    throw std::runtime_error("failed to create shader module!");
  }

  return shaderModule;
}

vk::PipelineLayout create_graphics_pipeline(vk::Device &device) {
  auto vertShaderCode = read_file("./compiled_shaders/shader__vert.spv");
  auto fragShaderCode = read_file("./compiled_shaders/shader__frag.spv");

  vk::ShaderModule vertShaderModule = create_shader_module(device, vertShaderCode);
  vk::ShaderModule fragShaderModule = create_shader_module(device, fragShaderCode);

  vk::PipelineShaderStageCreateInfo vertShaderStageInfo{};
  vertShaderStageInfo.stage = vk::ShaderStageFlagBits::eVertex;

  vertShaderStageInfo.module = vertShaderModule;
  vertShaderStageInfo.pName = "main";

  vk::PipelineShaderStageCreateInfo fragShaderStageInfo{};
  fragShaderStageInfo.stage = vk::ShaderStageFlagBits::eFragment;
  fragShaderStageInfo.module = fragShaderModule;
  fragShaderStageInfo.pName = "main";

  vk::PipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

  vk::PipelineLayout pipelineLayout;

  vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.setLayoutCount = 0;            // Optional
  pipelineLayoutInfo.pSetLayouts = nullptr;         // Optional
  pipelineLayoutInfo.pushConstantRangeCount = 0;    // Optional
  pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

  if (device.createPipelineLayout(&pipelineLayoutInfo, nullptr, &pipelineLayout) != vk::Result::eSuccess) {
    throw std::runtime_error("failed to create pipeline layout!");
  }

  // destroy shader modules
  device.destroyShaderModule(fragShaderModule, nullptr);
  device.destroyShaderModule(vertShaderModule, nullptr);

  return pipelineLayout;
}

std::vector<std::string> get_instance_available_extensions() {
  std::vector<std::string> names;
  std::vector<vk::ExtensionProperties> extensions = vk::enumerateInstanceExtensionProperties();

  for (const auto &extension : extensions) {
    names.push_back(extension.extensionName);
  }
  return names;
}

std::vector<std::string> get_physical_device_available_extensions(vk::PhysicalDevice &physicalDevice) {
  std::vector<std::string> names;
  for (const auto &extension : physicalDevice.enumerateDeviceExtensionProperties()) {
    names.push_back(extension.extensionName);
  }
  return names;
}

bool check_physical_device_extension_support(vk::PhysicalDevice &physicalDevice, std::vector<std::string> extensions) {

  for (auto extension : extensions) {
    auto avail = get_physical_device_available_extensions(physicalDevice);

    if (std::find(avail.begin(), avail.end(), extension) != avail.end()) {
      return true;
    }
  }
  return false;
}

vk::Buffer create_vertex_buffer(vk::Device &device, const std::vector<Vertex> &vertices, std::vector<uint16_t> indices) {
  // Create Vulkan buffers (vertex buffer, index buffer, etc.) ???
  vk::Buffer vertexBuffer;
  vk::BufferCreateInfo bufferInfo = {};
  bufferInfo.size = sizeof(Vertex) * vertices.size();
  bufferInfo.usage = vk::BufferUsageFlagBits::eVertexBuffer;

  device.createBuffer(&bufferInfo, nullptr, &vertexBuffer);

  return vertexBuffer;
}
} // namespace VK_TOOLS

// iostream utils

std::ostream &operator<<(std::ostream &os, const vk::PhysicalDeviceProperties &props) {
  os << "---------------------------\n";
  os << "vk::PhysicalDeviceProperties\n";
  os << "  device Name : " << props.deviceName << "\n";
  switch (props.deviceType) {
  case vk::PhysicalDeviceType::eIntegratedGpu:
    os << "  device Type : vk::PhysicalDeviceType::eIntegratedGpu\n";
    break;
  case vk::PhysicalDeviceType::eDiscreteGpu:
    os << "  device Type : vk::PhysicalDeviceType::eDiscreteGpu\n";
    break;
  default:
    os << "  device Type : Unknown\n";
    break;
  }
  return os;
}