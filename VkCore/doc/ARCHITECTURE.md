# VkCore 微型 Vulkan 渲染引擎架构设计

## 概述

VkCore 是一个轻量级的 Vulkan 渲染引擎核心，提供了对 Vulkan API 的高层封装，简化了 Vulkan 应用程序的开发流程。该引擎采用模块化设计，各组件职责清晰，易于扩展和维护。

## 核心设计原则

1. **模块化设计**：每个组件职责单一，接口清晰
2. **资源管理**：统一的资源创建、销毁和管理机制
3. **错误处理**：完善的错误检查和异常处理
4. **易用性**：提供高层接口，降低 Vulkan 开发门槛
5. **性能优化**：合理使用 Vulkan 特性，提升渲染性能

## 架构层次

```
┌─────────────────────────────────────┐
│              Renderer               │ ← 高级渲染接口
├─────────────────────────────────────┤
│         ResourceManager             │ ← 资源管理器
├─────────────────────────────────────┤
│   Pipeline │ Buffer │ Image │ ...   │ ← 核心组件
├─────────────────────────────────────┤
│      Device │ SwapChain │ ...       │ ← 基础组件
├─────────────────────────────────────┤
│           Instance                  │ ← Vulkan实例
└─────────────────────────────────────┘
```

## 核心组件详解

### 1. 基础层 (Foundation Layer)

#### VulkanInstance
- **职责**：管理 Vulkan 实例的创建、销毁和扩展
- **接口**：
  - `createInstance()` - 创建 Vulkan 实例
  - `createSurface(window)` - 创建窗口表面
  - `getInstance()` - 获取实例句柄
  - `getSurface()` - 获取表面句柄

#### VulkanDevice
- **职责**：管理物理设备选择和逻辑设备创建
- **接口**：
  - `pickPhysicalDevice()` - 选择合适的物理设备
  - `createLogicalDevice()` - 创建逻辑设备
  - `getDevice()` - 获取逻辑设备句柄
  - `getGraphicsQueue()` - 获取图形队列
  - `getPresentQueue()` - 获取呈现队列

### 2. 核心层 (Core Layer)

#### VulkanSwapChain
- **职责**：管理交换链的创建和重建
- **接口**：
  - `createSwapChain(width, height)` - 创建交换链
  - `getImageViews()` - 获取交换链图像视图
  - `recreate(width, height)` - 重建交换链
  - `acquireNextImage()` - 获取下一帧图像

#### VulkanRenderPass
- **职责**：管理渲染通道和帧缓冲区
- **接口**：
  - `createRenderPass()` - 创建渲染通道
  - `createFramebuffers()` - 创建帧缓冲区
  - `getRenderPass()` - 获取渲染通道句柄

#### VulkanPipeline
- **职责**：管理图形渲染管线
- **接口**：
  - `createGraphicsPipeline()` - 创建图形管线
  - `createDescriptorSetLayout()` - 创建描述符集布局
  - `getPipeline()` - 获取管线句柄
  - `getPipelineLayout()` - 获取管线布局

#### VulkanCommandBuffer
- **职责**：管理命令缓冲区的分配和提交
- **接口**：
  - `createCommandPool()` - 创建命令池
  - `allocateCommandBuffers(count)` - 分配命令缓冲区
  - `beginCommandBuffer(index)` - 开始记录命令
  - `endCommandBuffer(index)` - 结束记录命令
  - `submitCommandBuffer(index)` - 提交命令缓冲区

### 3. 资源层 (Resource Layer)

#### VulkanBuffer
- **职责**：管理各种类型的缓冲区（顶点、索引、Uniform等）
- **接口**：
  - `createVertexBuffer(vertices)` - 创建顶点缓冲区
  - `createIndexBuffer(indices)` - 创建索引缓冲区
  - `createUniformBuffer(size)` - 创建 Uniform 缓冲区
  - `updateUniformBuffer(data)` - 更新 Uniform 数据
  - `copyBuffer(src, dst, size)` - 缓冲区数据拷贝

#### VulkanImage
- **职责**：管理纹理图像和深度/颜色附件
- **接口**：
  - `createTextureImage(path)` - 从文件创建纹理
  - `createDepthImage(extent)` - 创建深度图像
  - `createColorImage(extent)` - 创建颜色图像
  - `getImageView()` - 获取图像视图
  - `getSampler()` - 获取采样器

#### VulkanDescriptorPool
- **职责**：管理描述符池和描述符集
- **接口**：
  - `createDescriptorPool(maxSets)` - 创建描述符池
  - `createDescriptorSets(count)` - 创建描述符集
  - `updateDescriptorSet(index, ...)` - 更新描述符集
  - `getDescriptorSet(index)` - 获取描述符集

### 4. 管理层 (Management Layer)

#### ResourceManager
- **职责**：统一管理所有 Vulkan 资源
- **接口**：
  - `createVertexBuffer(vertices)` - 创建顶点缓冲区
  - `createIndexBuffer(indices)` - 创建索引缓冲区
  - `createTexture(path)` - 创建纹理资源
  - `createDescriptorPool(pipeline)` - 创建描述符池
  - `cleanup()` - 清理所有资源

#### VulkanSyncObjects
- **职责**：管理同步对象（信号量、栅栏）
- **接口**：
  - `createSyncObjects(maxFrames)` - 创建同步对象
  - `getImageAvailableSemaphore(index)` - 获取图像可用信号量
  - `getRenderFinishedSemaphore(index)` - 获取渲染完成信号量
  - `getInFlightFence(index)` - 获取飞行中栅栏

### 5. 接口层 (Interface Layer)

#### Renderer
- **职责**：提供高层渲染接口，封装渲染流程
- **接口**：
  - `initialize(config, window)` - 初始化渲染器
  - `beginFrame()` - 开始新一帧渲染
  - `endFrame()` - 结束当前帧渲染
  - `draw(vertices, indices, ubo, texture)` - 执行绘制操作
  - `recreateSwapChain(width, height)` - 重建交换链
  - `waitIdle()` - 等待设备空闲

## 数据结构定义

### 配置结构
```cpp
struct VulkanConfig {
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
```

### 顶点结构
```cpp
struct Vertex {
    glm::vec3 position;
    glm::vec3 color;
    glm::vec2 texCoord;

    static VkVertexInputBindingDescription getBindingDescription();
    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions();
};
```

### Uniform 缓冲区
```cpp
struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};
```

### 队列族索引
```cpp
struct QueueFamilyIndices {
    uint32_t graphicsFamily = UINT32_MAX;
    uint32_t presentFamily = UINT32_MAX;

    bool isComplete() const {
        return graphicsFamily != UINT32_MAX && presentFamily != UINT32_MAX;
    }
};
```

## 组件依赖关系

```
Renderer
├── ResourceManager
├── VulkanSyncObjects
├── VulkanCommandBuffer
├── VulkanPipeline
├── VulkanRenderPass
├── VulkanSwapChain
├── VulkanDevice
└── VulkanInstance

ResourceManager
├── VulkanBuffer
├── VulkanImage
└── VulkanDescriptorPool

VulkanBuffer
├── VulkanDevice
└── VulkanCommandBuffer

VulkanImage
├── VulkanDevice
└── VulkanCommandBuffer

VulkanDescriptorPool
├── VulkanDevice
└── VulkanPipeline
```

## 文件组织结构

```
VkCore/
├── include/
│   ├── VkCore.h                 # 主头文件，包含所有类声明
│   ├── VulkanInstance.h         # 实例管理
│   ├── VulkanDevice.h           # 设备管理
│   ├── VulkanSwapChain.h        # 交换链管理
│   ├── VulkanRenderPass.h       # 渲染通道管理
│   ├── VulkanPipeline.h         # 管线管理
│   ├── VulkanCommandBuffer.h    # 命令缓冲区管理
│   ├── VulkanBuffer.h           # 缓冲区管理
│   ├── VulkanImage.h            # 图像管理
│   ├── VulkanDescriptor.h       # 描述符管理
│   ├── ResourceManager.h        # 资源管理器
│   ├── Renderer.h               # 渲染器接口
│   └── Types.h                  # 类型定义和数据结构
├── src/
│   ├── VkCore.cpp
│   ├── VulkanInstance.cpp
│   ├── VulkanDevice.cpp
│   ├── VulkanSwapChain.cpp
│   ├── VulkanRenderPass.cpp
│   ├── VulkanPipeline.cpp
│   ├── VulkanCommandBuffer.cpp
│   ├── VulkanBuffer.cpp
│   ├── VulkanImage.cpp
│   ├── VulkanDescriptor.cpp
│   ├── ResourceManager.cpp
│   ├── Renderer.cpp
│   └── Utils.cpp                # 工具函数
├── shaders/                     # 着色器文件
│   ├── vertex.vert
│   └── fragment.frag
├── ARCHITECTURE.md              # 架构设计文档
└── README.md                    # 使用说明
```

## 渲染流程

1. **初始化阶段**：
   ```cpp
   Renderer renderer(config, window);
   renderer.initialize();
   ```

2. **渲染循环**：
   ```cpp
   while (!glfwWindowShouldClose(window)) {
       renderer.beginFrame();

       // 应用层绘制逻辑
       renderer.draw(vertices, indices, ubo, texturePath);

       renderer.endFrame();
   }
   ```

3. **清理阶段**：
   ```cpp
   renderer.waitIdle();
   // 自动清理所有资源
   ```

## 扩展性设计

### 插件系统
- 通过接口抽象，允许自定义渲染管线
- 支持多种材质系统
- 可扩展的资源加载器

### 多线程支持
- 命令缓冲区支持并行录制
- 资源加载异步化
- 帧缓冲支持多线程渲染

### 调试支持
- 内置验证层支持
- 性能分析接口
- 错误日志记录

## 性能优化考虑

1. **内存管理**：
   - 统一内存分配策略
   - 缓冲区内存对齐优化
   - 纹理压缩支持

2. **渲染优化**：
   - 实例化渲染支持
   - 遮挡剔除接口
   - 多重采样抗锯齿

3. **资源管理**：
   - 资源引用计数
   - 延迟删除机制
   - 资源池复用

## 构建和部署

### 静态库构建
- **默认方式**: 构建为 `.lib` 静态库
- **优点**: 简单部署，无额外依赖
- **缺点**: 库文件较大，更新需重新编译

### DLL 构建
- **项目文件**: `VkCoreDLL.vcxproj`
- **导出宏**: 使用 `VkCoreAPI.h` 中的导出宏
- **优点**: 动态加载，支持插件系统
- **缺点**: 需要部署 DLL 文件

### 导出宏使用

```cpp
// 类声明
VKCORE_EXPORT_CLASS(MyClass) {
    VKCORE_FUNCTION void publicMethod();
private:
    void privateMethod();  // 不需要导出
};

// 结构体声明
VKCORE_EXPORT_STRUCT(MyStruct) {
    int value;
};

// 全局函数
VKCORE_EXPORT_FUNCTION(void, globalFunction)(int param);
```

## 开发指南

### 命名约定
- 类名：`VulkanXxx` 格式
- 方法名：驼峰命名法
- 私有成员：`xxx_` 格式（下划线结尾）

### 错误处理
- 使用异常处理 Vulkan 错误
- 提供详细的错误信息
- 支持调试模式下的断言

### 文档要求
- 每个公共接口都需要文档注释
- 包含使用示例
- 说明参数和返回值含义

这个架构设计提供了 Vulkan 渲染引擎的核心框架，你可以根据具体需求进行实现和扩展。