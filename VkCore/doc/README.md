# VkCore - 微型 Vulkan 渲染引擎

VkCore 是一个轻量级的 Vulkan 渲染引擎核心，提供对 Vulkan API 的高层封装，简化 Vulkan 应用程序开发。

## 快速开始

### 1. 初始化渲染器

```cpp
#include "Renderer.h"

// 配置 Vulkan 设置
VulkanConfig config;
config.applicationName = "My Vulkan App";
config.windowWidth = 1280;
config.windowHeight = 720;
config.enableValidationLayers = true; // 开发时启用

// 创建渲染器
Renderer renderer(config, window);
renderer.initialize();
```

### 2. 渲染循环

```cpp
// 定义顶点数据
std::vector<Vertex> vertices = {
    {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
    {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
    {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
    {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
};

std::vector<uint32_t> indices = {0, 1, 2, 2, 3, 0};

// Uniform 缓冲区数据
UniformBufferObject ubo = {};
ubo.model = glm::mat4(1.0f);
ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
ubo.proj = glm::perspective(glm::radians(45.0f), 1280.0f / 720.0f, 0.1f, 10.0f);

// 主循环
while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    renderer.beginFrame();

    // 绘制三角形
    renderer.draw(vertices, indices, ubo, "textures/texture.jpg");

    renderer.endFrame();
}

renderer.waitIdle();
```

### 3. 高级用法

```cpp
// 获取底层 Vulkan 对象进行自定义操作
VulkanDevice& device = renderer.getDevice();
VulkanSwapChain& swapChain = renderer.getSwapChain();

// 创建自定义缓冲区
auto customBuffer = std::make_shared<VulkanBuffer>(device, renderer.getCommandBuffer());
customBuffer->createUniformBuffer(sizeof(MyUniformData));

// 创建自定义纹理
auto customTexture = std::make_shared<VulkanImage>(device, renderer.getCommandBuffer());
customTexture->createTextureImage("path/to/texture.png");
```

## 架构概述

VkCore 采用分层架构设计：

### 1. 基础层 (Foundation)
- `VulkanInstance` - Vulkan 实例管理
- `VulkanDevice` - 物理/逻辑设备管理

### 2. 核心层 (Core)
- `VulkanSwapChain` - 交换链管理
- `VulkanRenderPass` - 渲染通道管理
- `VulkanPipeline` - 图形管线管理
- `VulkanCommandBuffer` - 命令缓冲区管理

### 3. 资源层 (Resource)
- `VulkanBuffer` - 缓冲区管理 (顶点、索引、Uniform)
- `VulkanImage` - 图像管理 (纹理、深度缓冲)
- `VulkanDescriptorPool` - 描述符管理

### 4. 管理层 (Management)
- `ResourceManager` - 统一资源管理
- `VulkanSyncObjects` - 同步对象管理

### 5. 接口层 (Interface)
- `Renderer` - 高层渲染接口

## 特性

- ✅ 现代 Vulkan API 支持 (1.3)
- ✅ 自动资源管理
- ✅ 交换链重建支持
- ✅ MSAA 多重采样
- ✅ 纹理映射
- ✅ Uniform 缓冲区
- ✅ 验证层调试支持
- ✅ GLFW 窗口集成
- ✅ GLM 数学库集成

## 构建要求

- Vulkan SDK
- GLFW 库
- GLM 数学库
- C++17 编译器

## 扩展开发

VkCore 设计时考虑了扩展性：

### 自定义渲染管线
```cpp
class CustomPipeline : public VulkanPipeline {
    // 实现自定义管线逻辑
};
```

### 自定义资源加载器
```cpp
class CustomTextureLoader : public TextureLoader {
    // 实现自定义纹理加载
};
```

### 多线程渲染
```cpp
// 支持命令缓冲区并行录制
renderer.enableMultiThreading(true);
```

## 性能优化

- 延迟删除机制减少卡顿
- 资源池复用减少内存分配
- 支持实例化渲染
- 可配置的多重采样

## 调试支持

```cpp
// 启用详细日志
config.enableValidationLayers = true;
config.enableDebugMarkers = true;

// 性能分析
renderer.enableProfiling(true);
```

## 示例项目

查看 `samples/` 目录中的示例：
- `HelloVulkan` - 基础 Vulkan 初始化
- `VulkanSandbox` - VkCore 使用示例

## 许可证

本项目采用 MIT 许可证。