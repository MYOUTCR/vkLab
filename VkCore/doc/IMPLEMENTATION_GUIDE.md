# VkCore 实现指南

本指南将详细说明如何实现 VkCore 微型 Vulkan 渲染引擎的各个组件。

## 目录

1. [准备工作](#准备工作)
2. [基础组件实现](#基础组件实现)
3. [核心组件实现](#核心组件实现)
4. [资源管理实现](#资源管理实现)
5. [渲染器实现](#渲染器实现)
6. [完整使用示例](#完整使用示例)

## 准备工作

### 1. 项目设置

首先更新你的 `pch.h` 包含必要的头文件：

```cpp
// pch.h
#ifndef PCH_H
#define PCH_H

#include "framework.h"

// Vulkan 和 GLFW
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// GLM 数学库
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// 标准库
#include <iostream>
#include <stdexcept>
#include <vector>
#include <array>
#include <set>
#include <optional>
#include <algorithm>
#include <fstream>
#include <string>
#include <memory>
#include <chrono>

// 定义
#define VK_CHECK(result) if (result != VK_SUCCESS) throw std::runtime_error("Vulkan error: " + std::to_string(result));

#endif //PCH_H
```

### 2. 导出宏配置

创建 `VkCoreAPI.h` 定义导出宏（用于 DLL 构建）：

```cpp
// VkCoreAPI.h
#pragma once

/**
 * VkCore API 导出宏定义
 */

// 平台检测
#ifdef _WIN32
    #ifdef VKCORE_EXPORTS
        #define VKCORE_API __declspec(dllexport)
    #else
        #define VKCORE_API __declspec(dllimport)
    #endif
#else
    #define VKCORE_API __attribute__((visibility("default")))
#endif

// 导出控制宏
#define VKCORE_EXPORT VKCORE_API
#define VKCORE_CLASS VKCORE_API
#define VKCORE_FUNCTION VKCORE_API

// 便捷宏
#define VKCORE_EXPORT_CLASS(className) class VKCORE_CLASS className
#define VKCORE_EXPORT_STRUCT(structName) struct VKCORE_CLASS structName
#define VKCORE_EXPORT_FUNCTION(returnType, functionName) VKCORE_FUNCTION returnType functionName
```

**DLL 构建配置：**
1. 在 VkCore 项目属性中定义 `VKCORE_EXPORTS` 预处理器宏
2. 使用示例项目引用时不需要此宏（自动使用 dllimport）

### 3. 类型定义

创建 `Types.h` 定义基础数据结构：

```cpp
// Types.h
#pragma once

#include "VkCoreAPI.h"  // 导出宏定义
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <vector>

// Vulkan 配置
VKCORE_EXPORT_STRUCT(VulkanConfig) {
    std::string applicationName = "VkCore Application";
    uint32_t applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    uint32_t apiVersion = VK_API_VERSION_1_3;

    uint32_t windowWidth = 800;
    uint32_t windowHeight = 600;
    bool enableValidationLayers = true;

    bool enableAnisotropy = true;
    bool enableSampleShading = true;
    uint32_t maxMSAASamples = 8;
};

// 顶点结构
struct Vertex {
    glm::vec3 position;
    glm::vec3 color;
    glm::vec2 texCoord;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, position);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

        return attributeDescriptions;
    }

    bool operator==(const Vertex& other) const {
        return position == other.position && color == other.color && texCoord == other.texCoord;
    }
};

// Uniform 缓冲区对象
VKCORE_EXPORT_STRUCT(UniformBufferObject) {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

// 队列族索引
VKCORE_EXPORT_STRUCT(QueueFamilyIndices) {
    uint32_t graphicsFamily = UINT32_MAX;
    uint32_t presentFamily = UINT32_MAX;

    bool isComplete() const {
        return graphicsFamily != UINT32_MAX && presentFamily != UINT32_MAX;
    }
};

// 工具函数
std::vector<char> readFile(const std::string& filename);
void checkVkResult(VkResult result, const std::string& message = "");
```

### 3. 工具函数实现

创建 `Utils.cpp` 实现工具函数：

```cpp
// Utils.cpp
#include "Types.h"

std::vector<char> readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filename);
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}

void checkVkResult(VkResult result, const std::string& message) {
    if (result != VK_SUCCESS) {
        throw std::runtime_error(message + " (Vulkan error: " + std::to_string(result) + ")");
    }
}
```

## 基础组件实现

### 1. VulkanInstance 实现

```cpp
// VulkanInstance.h
#pragma once
#include "Types.h"
#include "VkCoreAPI.h"  // 导出宏
#include <GLFW/glfw3.h>

VKCORE_EXPORT_CLASS(VulkanInstance) {
public:
    VulkanInstance(const VulkanConfig& config);
    ~VulkanInstance();

    VkInstance getInstance() const { return instance_; }
    VkSurfaceKHR getSurface() const { return surface_; }
    const VulkanConfig& getConfig() const { return config_; }

    void createSurface(GLFWwindow* window);

private:
    VkInstance instance_ = VK_NULL_HANDLE;
    VkSurfaceKHR surface_ = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT debugMessenger_ = VK_NULL_HANDLE;

    const VulkanConfig& config_;
    std::vector<const char*> validationLayers_;
    std::vector<const char*> instanceExtensions_;

    void createInstance();
    void setupDebugMessenger();
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    std::vector<const char*> getRequiredExtensions();
    bool checkValidationLayerSupport() const;
};

// VulkanInstance.cpp
#include "VulkanInstance.h"

VulkanInstance::VulkanInstance(const VulkanConfig& config) : config_(config) {
    validationLayers_ = {"VK_LAYER_KHRONOS_validation"};
    instanceExtensions_ = {VK_EXT_DEBUG_UTILS_EXTENSION_NAME};
    createInstance();
    if (config_.enableValidationLayers) {
        setupDebugMessenger();
    }
}

VulkanInstance::~VulkanInstance() {
    if (debugMessenger_ != VK_NULL_HANDLE) {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance_, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) {
            func(instance_, debugMessenger_, nullptr);
        }
    }
    if (surface_ != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(instance_, surface_, nullptr);
    }
    if (instance_ != VK_NULL_HANDLE) {
        vkDestroyInstance(instance_, nullptr);
    }
}

void VulkanInstance::createInstance() {
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = config_.applicationName.c_str();
    appInfo.applicationVersion = config_.applicationVersion;
    appInfo.pEngineName = "VkCore";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = config_.apiVersion;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    auto extensions = getRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (config_.enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers_.size());
        createInfo.ppEnabledLayerNames = validationLayers_.data();
        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    } else {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }

    VK_CHECK(vkCreateInstance(&createInfo, nullptr, &instance_));
}

void VulkanInstance::createSurface(GLFWwindow* window) {
    VK_CHECK(glfwCreateWindowSurface(instance_, window, nullptr, &surface_));
}

std::vector<const char*> VulkanInstance::getRequiredExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (config_.enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

bool VulkanInstance::checkValidationLayerSupport() const {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : validationLayers_) {
        bool layerFound = false;
        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }
        if (!layerFound) return false;
    }
    return true;
}

void VulkanInstance::setupDebugMessenger() {
    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    populateDebugMessengerCreateInfo(createInfo);

    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance_, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        VK_CHECK(func(instance_, &createInfo, nullptr, &debugMessenger_));
    }
}

void VulkanInstance::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = [](VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                    VkDebugUtilsMessageTypeFlagsEXT messageType,
                                    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                    void* pUserData) -> VkBool32 {
        std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;
        return VK_FALSE;
    };
}
```

### 2. VulkanDevice 实现

```cpp
// VulkanDevice.h
#pragma once
#include "VulkanInstance.h"

class VulkanDevice {
public:
    VulkanDevice(VulkanInstance& instance);
    ~VulkanDevice();

    VkPhysicalDevice getPhysicalDevice() const { return physicalDevice_; }
    VkDevice getDevice() const { return device_; }
    VkQueue getGraphicsQueue() const { return graphicsQueue_; }
    VkQueue getPresentQueue() const { return presentQueue_; }
    uint32_t getGraphicsQueueFamily() const { return graphicsQueueFamily_; }
    uint32_t getPresentQueueFamily() const { return presentQueueFamily_; }

    VkSampleCountFlagBits getMaxUsableSampleCount() const;
    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates,
                                VkImageTiling tiling, VkFormatFeatureFlags features) const;
    VkFormat findDepthFormat() const;
    bool hasStencilComponent(VkFormat format) const;

private:
    VulkanInstance& instance_;
    VkPhysicalDevice physicalDevice_ = VK_NULL_HANDLE;
    VkDevice device_ = VK_NULL_HANDLE;

    VkQueue graphicsQueue_ = VK_NULL_HANDLE;
    VkQueue presentQueue_ = VK_NULL_HANDLE;
    uint32_t graphicsQueueFamily_ = 0;
    uint32_t presentQueueFamily_ = 0;

    void pickPhysicalDevice();
    void createLogicalDevice();
    bool isDeviceSuitable(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    std::vector<const char*> getDeviceExtensions();
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
};

// VulkanDevice.cpp
#include "VulkanDevice.h"

VulkanDevice::VulkanDevice(VulkanInstance& instance) : instance_(instance) {
    pickPhysicalDevice();
    createLogicalDevice();
}

VulkanDevice::~VulkanDevice() {
    if (device_ != VK_NULL_HANDLE) {
        vkDestroyDevice(device_, nullptr);
    }
}

void VulkanDevice::pickPhysicalDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance_.getInstance(), &deviceCount, nullptr);

    if (deviceCount == 0) {
        throw std::runtime_error("Failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance_.getInstance(), &deviceCount, devices.data());

    for (const auto& device : devices) {
        if (isDeviceSuitable(device)) {
            physicalDevice_ = device;
            break;
        }
    }

    if (physicalDevice_ == VK_NULL_HANDLE) {
        throw std::runtime_error("Failed to find a suitable GPU!");
    }
}

bool VulkanDevice::isDeviceSuitable(VkPhysicalDevice device) {
    QueueFamilyIndices indices = findQueueFamilies(device);

    bool extensionsSupported = checkDeviceExtensionSupport(device);

    bool swapChainAdequate = false;
    if (extensionsSupported) {
        // 这里可以添加更多的检查，比如交换链支持等
        swapChainAdequate = true;
    }

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

    return indices.isComplete() && extensionsSupported && swapChainAdequate &&
           (instance_.getConfig().enableAnisotropy ? supportedFeatures.samplerAnisotropy : true);
}

bool VulkanDevice::checkDeviceExtensionSupport(VkPhysicalDevice device) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(getDeviceExtensions().begin(), getDeviceExtensions().end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

std::vector<const char*> VulkanDevice::getDeviceExtensions() {
    return {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
}

QueueFamilyIndices VulkanDevice::findQueueFamilies(VkPhysicalDevice device) {
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, instance_.getSurface(), &presentSupport);
        if (presentSupport) {
            indices.presentFamily = i;
        }

        if (indices.isComplete()) {
            break;
        }
        i++;
    }

    return indices;
}

void VulkanDevice::createLogicalDevice() {
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice_);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily, indices.presentFamily};

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = instance_.getConfig().enableAnisotropy ? VK_TRUE : VK_FALSE;
    deviceFeatures.sampleRateShading = instance_.getConfig().enableSampleShading ? VK_TRUE : VK_FALSE;

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;

    auto deviceExtensions = getDeviceExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (instance_.getConfig().enableValidationLayers) {
        std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    VK_CHECK(vkCreateDevice(physicalDevice_, &createInfo, nullptr, &device_));

    vkGetDeviceQueue(device_, indices.graphicsFamily, 0, &graphicsQueue_);
    vkGetDeviceQueue(device_, indices.presentFamily, 0, &presentQueue_);

    graphicsQueueFamily_ = indices.graphicsFamily;
    presentQueueFamily_ = indices.presentFamily;
}

VkSampleCountFlagBits VulkanDevice::getMaxUsableSampleCount() const {
    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice_, &physicalDeviceProperties);

    VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts &
                                physicalDeviceProperties.limits.framebufferDepthSampleCounts;

    uint32_t maxSamples = instance_.getConfig().maxMSAASamples;

    if (counts & VK_SAMPLE_COUNT_64_BIT && maxSamples >= 64) return VK_SAMPLE_COUNT_64_BIT;
    if (counts & VK_SAMPLE_COUNT_32_BIT && maxSamples >= 32) return VK_SAMPLE_COUNT_32_BIT;
    if (counts & VK_SAMPLE_COUNT_16_BIT && maxSamples >= 16) return VK_SAMPLE_COUNT_16_BIT;
    if (counts & VK_SAMPLE_COUNT_8_BIT && maxSamples >= 8) return VK_SAMPLE_COUNT_8_BIT;
    if (counts & VK_SAMPLE_COUNT_4_BIT && maxSamples >= 4) return VK_SAMPLE_COUNT_4_BIT;
    if (counts & VK_SAMPLE_COUNT_2_BIT && maxSamples >= 2) return VK_SAMPLE_COUNT_2_BIT;

    return VK_SAMPLE_COUNT_1_BIT;
}

VkFormat VulkanDevice::findSupportedFormat(const std::vector<VkFormat>& candidates,
                                          VkImageTiling tiling, VkFormatFeatureFlags features) const {
    for (VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physicalDevice_, format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return format;
        } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    throw std::runtime_error("Failed to find supported format!");
}

VkFormat VulkanDevice::findDepthFormat() const {
    return findSupportedFormat(
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

bool VulkanDevice::hasStencilComponent(VkFormat format) const {
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}
```

## 核心组件实现

### 3. VulkanSwapChain 实现

```cpp
// VulkanSwapChain.h
#pragma once
#include "VulkanDevice.h"

class VulkanSwapChain {
public:
    VulkanSwapChain(VulkanDevice& device, VulkanInstance& instance, uint32_t width, uint32_t height);
    ~VulkanSwapChain();

    VkSwapchainKHR getSwapChain() const { return swapChain_; }
    VkFormat getImageFormat() const { return imageFormat_; }
    VkExtent2D getExtent() const { return extent_; }
    const std::vector<VkImageView>& getImageViews() const { return imageViews_; }
    size_t getImageCount() const { return images_.size(); }

    VkResult acquireNextImage(uint32_t& imageIndex, VkSemaphore semaphore);
    void recreate(uint32_t width, uint32_t height);

private:
    VulkanDevice& device_;
    VulkanInstance& instance_;

    VkSwapchainKHR swapChain_ = VK_NULL_HANDLE;
    std::vector<VkImage> images_;
    std::vector<VkImageView> imageViews_;
    VkFormat imageFormat_;
    VkExtent2D extent_;

    void createSwapChain(uint32_t width, uint32_t height);
    void createImageViews();
    void cleanup();

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, uint32_t width, uint32_t height);
};

// VulkanSwapChain.cpp
#include "VulkanSwapChain.h"

VulkanSwapChain::VulkanSwapChain(VulkanDevice& device, VulkanInstance& instance, uint32_t width, uint32_t height)
    : device_(device), instance_(instance) {
    createSwapChain(width, height);
    createImageViews();
}

VulkanSwapChain::~VulkanSwapChain() {
    cleanup();
}

void VulkanSwapChain::createSwapChain(uint32_t width, uint32_t height) {
    // 获取交换链支持详情
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device_.getPhysicalDevice(), instance_.getSurface(), &capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device_.getPhysicalDevice(), instance_.getSurface(), &formatCount, nullptr);
    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device_.getPhysicalDevice(), instance_.getSurface(), &formatCount, formats.data());

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device_.getPhysicalDevice(), instance_.getSurface(), &presentModeCount, nullptr);
    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device_.getPhysicalDevice(), instance_.getSurface(), &presentModeCount, presentModes.data());

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(presentModes);
    VkExtent2D extent = chooseSwapExtent(capabilities, width, height);

    uint32_t imageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
        imageCount = capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = instance_.getSurface();
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    uint32_t queueFamilyIndices[] = {device_.getGraphicsQueueFamily(), device_.getPresentQueueFamily()};
    if (device_.getGraphicsQueueFamily() != device_.getPresentQueueFamily()) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }

    createInfo.preTransform = capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    VK_CHECK(vkCreateSwapchainKHR(device_.getDevice(), &createInfo, nullptr, &swapChain_));

    vkGetSwapchainImagesKHR(device_.getDevice(), swapChain_, &imageCount, nullptr);
    images_.resize(imageCount);
    vkGetSwapchainImagesKHR(device_.getDevice(), swapChain_, &imageCount, images_.data());

    imageFormat_ = surfaceFormat.format;
    extent_ = extent;
}

void VulkanSwapChain::createImageViews() {
    imageViews_.resize(images_.size());

    for (size_t i = 0; i < images_.size(); i++) {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = images_[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = imageFormat_;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        VK_CHECK(vkCreateImageView(device_.getDevice(), &createInfo, nullptr, &imageViews_[i]));
    }
}

void VulkanSwapChain::cleanup() {
    for (auto imageView : imageViews_) {
        vkDestroyImageView(device_.getDevice(), imageView, nullptr);
    }
    imageViews_.clear();

    if (swapChain_ != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(device_.getDevice(), swapChain_, nullptr);
        swapChain_ = VK_NULL_HANDLE;
    }
}

VkResult VulkanSwapChain::acquireNextImage(uint32_t& imageIndex, VkSemaphore semaphore) {
    return vkAcquireNextImageKHR(device_.getDevice(), swapChain_, UINT64_MAX, semaphore, VK_NULL_HANDLE, &imageIndex);
}

void VulkanSwapChain::recreate(uint32_t width, uint32_t height) {
    vkDeviceWaitIdle(device_.getDevice());
    cleanup();
    createSwapChain(width, height);
    createImageViews();
}

VkSurfaceFormatKHR VulkanSwapChain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }
    return availableFormats[0];
}

VkPresentModeKHR VulkanSwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanSwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, uint32_t width, uint32_t height) {
    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    } else {
        VkExtent2D actualExtent = {width, height};
        actualExtent.width = std::max(capabilities.minImageExtent.width,
                                     std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(capabilities.minImageExtent.height,
                                      std::min(capabilities.maxImageExtent.height, actualExtent.height));
        return actualExtent;
    }
}
```

## 完整使用示例

### 基础三角形渲染示例

创建 `samples/TriangleExample/TriangleExample.cpp`：

```cpp
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>

// VkCore 头文件
#include "Renderer.h"

const std::vector<Vertex> vertices = {
    {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
    {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
    {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
    {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
};

const std::vector<uint32_t> indices = {
    0, 1, 2, 2, 3, 0
};

int main() {
    // 初始化 GLFW
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "VkCore Triangle Example", nullptr, nullptr);

    // VkCore 配置
    VulkanConfig config;
    config.applicationName = "Triangle Example";
    config.windowWidth = 800;
    config.windowHeight = 600;

    // 创建渲染器
    Renderer renderer(config, window);
    renderer.initialize();

    // 主循环
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // 更新 Uniform 数据
        static auto startTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        UniformBufferObject ubo{};
        ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.proj = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 10.0f);
        ubo.proj[1][1] *= -1; // GLM 的 Y 轴与 Vulkan 相反

        // 渲染帧
        renderer.beginFrame();
        renderer.draw(vertices, indices, ubo);
        renderer.endFrame();
    }

    renderer.waitIdle();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
```

### 纹理渲染示例

创建 `samples/TextureExample/TextureExample.cpp`：

```cpp
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// VkCore 头文件
#include "Renderer.h"

const std::vector<Vertex> vertices = {
    {{-0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
    {{0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
    {{0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
    {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
};

const std::vector<uint32_t> indices = {
    0, 1, 2, 2, 3, 0
};

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(800, 600, "VkCore Texture Example", nullptr, nullptr);

    VulkanConfig config;
    config.applicationName = "Texture Example";

    Renderer renderer(config, window);
    renderer.initialize();

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        UniformBufferObject ubo{};
        ubo.model = glm::mat4(1.0f);
        ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.proj = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 10.0f);
        ubo.proj[1][1] *= -1;

        renderer.beginFrame();
        // 渲染带纹理的四边形
        renderer.draw(vertices, indices, ubo, "textures/container.jpg");
        renderer.endFrame();
    }

    renderer.waitIdle();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
```

## 着色器实现

### 顶点着色器 (shader.vert)

```glsl
#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
    fragColor = inColor;
    fragTexCoord = inTexCoord;
}
```

### 片段着色器 (shader.frag)

```glsl
#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler;

void main() {
    outColor = texture(texSampler, fragTexCoord);
}
```

## 构建说明

1. **编译着色器**：使用 glslc 编译着色器
   ```bash
   glslc shader.vert -o vert.spv
   glslc shader.frag -o frag.spv
   ```

2. **链接库**：确保链接 Vulkan、GLFW 和 GLM 库

3. **包含路径**：设置正确的包含路径以包含 VkCore 头文件

这个实现指南提供了 VkCore 的完整实现框架。按照这个顺序实现各个组件，你就能构建出一个功能完整的微型 Vulkan 渲染引擎。