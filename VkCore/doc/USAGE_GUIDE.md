# VkCore 使用指南

本指南将详细说明如何在你的项目中使用 VkCore 微型 Vulkan 渲染引擎。

## 目录

1. [快速开始](#快速开始)
2. [项目设置](#项目设置)
3. [基础渲染](#基础渲染)
4. [高级功能](#高级功能)
5. [自定义扩展](#自定义扩展)
6. [性能优化](#性能优化)
7. [调试技巧](#调试技巧)

## 快速开始

### 1. 包含头文件

```cpp
#include "Renderer.h"  // 主渲染器接口
```

### 2. 配置和初始化

```cpp
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

### 3. 渲染循环

```cpp
// 定义顶点数据
std::vector<Vertex> vertices = {
    {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
    {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
    {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
    {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
};
std::vector<uint32_t> indices = {0, 1, 2, 2, 3, 0};

// 主循环
while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    // 计算变换矩阵
    UniformBufferObject ubo{};
    ubo.model = glm::mat4(1.0f);
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj = glm::perspective(glm::radians(45.0f), 1280.0f / 720.0f, 0.1f, 10.0f);
    ubo.proj[1][1] *= -1;

    // 渲染
    renderer.beginFrame();
    renderer.draw(vertices, indices, ubo, "texture.jpg");
    renderer.endFrame();
}

// 清理
renderer.waitIdle();
```

## 项目设置

### Visual Studio 配置

1. **添加包含目录**：
   - 项目属性 → C/C++ → 常规 → 附加包含目录
   - 添加 `$(SolutionDir)VkCore`

2. **添加库依赖**：
   - 项目属性 → 链接器 → 输入 → 附加依赖项
   - 添加 `VkCore.lib`

3. **添加库目录**：
   - 项目属性 → 链接器 → 常规 → 附加库目录
   - 添加 `$(SolutionDir)x64\$(Configuration)`

### 依赖库

确保你的项目链接了以下库：
- `vulkan-1.lib` - Vulkan API
- `glfw3.lib` - GLFW 窗口管理
- GLM 数学库（头文件包含）

### DLL 构建 (可选)

VkCore 支持 DLL 构建，便于动态加载和插件系统：

1. **使用 DLL 项目**：
   - 引用 `VkCoreDLL.vcxproj` 项目
   - 构建生成 `VkCore.dll` 和导入库

2. **运行时要求**：
   - 将 `VkCore.dll` 放在可执行文件目录
   - 确保 Vulkan 运行时可用

3. **使用示例**：
   ```cpp
   // DLL 版本使用方式相同
   #include "Renderer.h"

   VulkanConfig config;
   Renderer renderer(config, window); // 自动从 DLL 导入
   ```

### 着色器编译

使用 Vulkan SDK 中的 `glslc` 编译着色器：

```bash
# 编译顶点着色器
glslc shader.vert -o vert.spv

# 编译片段着色器
glslc shader.frag -o frag.spv
```

## 基础渲染

### 顶点数据结构

VkCore 使用标准的顶点格式：

```cpp
struct Vertex {
    glm::vec3 position;  // 位置
    glm::vec3 color;     // 颜色
    glm::vec2 texCoord;  // 纹理坐标
};
```

### 变换矩阵

Uniform 缓冲区包含 MVP 矩阵：

```cpp
struct UniformBufferObject {
    glm::mat4 model;  // 模型矩阵
    glm::mat4 view;   // 视图矩阵
    glm::mat4 proj;   // 投影矩阵
};
```

### 基本几何体

#### 三角形
```cpp
std::vector<Vertex> triangleVertices = {
    {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
    {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
    {{0.0f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.5f, 1.0f}}
};
std::vector<uint32_t> triangleIndices = {0, 1, 2};
```

#### 四边形
```cpp
std::vector<Vertex> quadVertices = {
    {{-0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
    {{0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
    {{0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
    {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
};
std::vector<uint32_t> quadIndices = {0, 1, 2, 2, 3, 0};
```

#### 立方体
```cpp
// 参考 samples/AdvancedExample/ 中的 cubeVertices
```

## 高级功能

### 多个对象的渲染

```cpp
struct RenderObject {
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
    std::string texturePath;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
};

std::vector<RenderObject> objects = {
    // 对象1
    {
        glm::vec3(-1.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(1.0f, 1.0f, 1.0f),
        "texture1.jpg",
        quadVertices,
        quadIndices
    },
    // 对象2
    {
        glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(1.0f, 1.0f, 1.0f),
        "texture2.jpg",
        quadVertices,
        quadIndices
    }
};

// 在渲染循环中
for (const auto& obj : objects) {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, obj.position);
    model = glm::rotate(model, obj.rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, obj.rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, obj.rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, obj.scale);

    UniformBufferObject ubo{};
    ubo.model = model;
    ubo.view = viewMatrix;
    ubo.proj = projMatrix;

    renderer.draw(obj.vertices, obj.indices, ubo, obj.texturePath);
}
```

### 动画和变换

#### 旋转动画
```cpp
float time = /* 获取当前时间 */;
glm::mat4 model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
```

#### 相机控制
```cpp
// 环绕相机
float radius = 5.0f;
glm::vec3 cameraPos = glm::vec3(
    sin(time) * radius,
    2.0f,
    cos(time) * radius
);
glm::mat4 view = glm::lookAt(cameraPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
```

#### 透视投影
```cpp
glm::mat4 proj = glm::perspective(
    glm::radians(45.0f),     // FOV
    width / (float)height,   // 宽高比
    0.1f,                    // 近裁剪面
    100.0f                   // 远裁剪面
);
proj[1][1] *= -1; // Vulkan Y轴翻转
```

### 纹理使用

```cpp
// 加载纹理（自动处理）
renderer.draw(vertices, indices, ubo, "path/to/texture.jpg");

// 不使用纹理（使用顶点颜色）
renderer.draw(vertices, indices, ubo, ""); // 空字符串
```

## 自定义扩展

### 自定义渲染管线

```cpp
class CustomRenderer : public Renderer {
public:
    CustomRenderer(const VulkanConfig& config, GLFWwindow* window)
        : Renderer(config, window) {}

    // 重写渲染方法
    void renderScene() {
        beginFrame();

        // 你的自定义渲染逻辑
        drawCustomObjects();
        drawParticles();
        drawUI();

        endFrame();
    }

private:
    void drawCustomObjects() {
        // 自定义对象渲染
    }

    void drawParticles() {
        // 粒子系统渲染
    }

    void drawUI() {
        // UI 渲染
    }
};
```

### 自定义顶点格式

```cpp
struct CustomVertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
    glm::vec4 tangent;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(CustomVertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions() {
        // 实现属性描述
    }
};
```

### 自定义材质系统

```cpp
struct Material {
    std::string diffuseTexture;
    std::string normalTexture;
    std::string specularTexture;
    glm::vec3 diffuseColor;
    float specularPower;
    float roughness;
};

class MaterialRenderer {
public:
    void renderWithMaterial(const std::vector<Vertex>& vertices,
                           const std::vector<uint32_t>& indices,
                           const UniformBufferObject& ubo,
                           const Material& material) {
        // 设置材质参数
        // 绑定纹理
        // 更新 Uniform 缓冲区
        // 调用渲染
    }
};
```

## 性能优化

### 批处理渲染

```cpp
// 将多个相似对象组合成一个大的顶点缓冲区
std::vector<Vertex> batchedVertices;
std::vector<uint32_t> batchedIndices;

// 添加多个实例
for (const auto& obj : objects) {
    // 变换顶点并添加到批处理缓冲区
    for (const auto& vertex : obj.vertices) {
        Vertex transformedVertex = vertex;
        // 应用对象变换
        batchedVertices.push_back(transformedVertex);
    }
    // 调整索引偏移
}

// 单次绘制调用
renderer.draw(batchedVertices, batchedIndices, ubo, texture);
```

### 纹理图集

```cpp
// 使用纹理图集减少纹理切换
class TextureAtlas {
public:
    void loadAtlas(const std::string& atlasPath);
    glm::vec4 getUVRect(const std::string& textureName);
};

// 更新纹理坐标
for (auto& vertex : vertices) {
    glm::vec4 uvRect = atlas.getUVRect("texture1");
    vertex.texCoord = glm::vec2(
        uvRect.x + vertex.texCoord.x * uvRect.z,
        uvRect.y + vertex.texCoord.y * uvRect.w
    );
}
```

### 视锥剔除

```cpp
bool isVisible(const glm::vec3& position, float radius, const glm::mat4& viewProj) {
    // 简单的视锥剔除
    glm::vec4 clipPos = viewProj * glm::vec4(position, 1.0f);
    clipPos /= clipPos.w;

    return clipPos.x >= -1.0f - radius && clipPos.x <= 1.0f + radius &&
           clipPos.y >= -1.0f - radius && clipPos.y <= 1.0f + radius &&
           clipPos.z >= -1.0f && clipPos.z <= 1.0f;
}

// 在渲染前检查可见性
if (isVisible(obj.position, obj.boundingRadius, viewProjMatrix)) {
    renderer.draw(obj.vertices, obj.indices, ubo, obj.texture);
}
```

## 调试技巧

### 验证层

```cpp
// 启用验证层
VulkanConfig config;
config.enableValidationLayers = true;

// 在 Debug 模式下会输出详细的 Vulkan API 使用信息
```

### 帧率显示

```cpp
static double lastTime = glfwGetTime();
static int frameCount = 0;

double currentTime = glfwGetTime();
frameCount++;

if (currentTime - lastTime >= 1.0) {
    std::cout << "FPS: " << frameCount << std::endl;
    frameCount = 0;
    lastTime = currentTime;
}
```

### 错误处理

```cpp
try {
    Renderer renderer(config, window);
    renderer.initialize();

    while (!glfwWindowShouldClose(window)) {
        // 渲染循环
    }

    renderer.waitIdle();
} catch (const std::exception& e) {
    std::cerr << "渲染错误: " << e.what() << std::endl;
    return EXIT_FAILURE;
}
```

### 内存泄漏检测

```cpp
// 在程序结束前检查 Vulkan 对象是否正确清理
renderer.waitIdle(); // 确保所有操作完成

// 使用 Vulkan 内存分配器回调来跟踪内存使用
// 或使用第三方内存泄漏检测工具
```

## 示例项目

查看以下示例项目了解具体用法：

- **`HelloVulkan`** - 基础四边形渲染
- **`AdvancedExample`** - 多对象、动画、3D 场景
- **`VulkanSandbox`** - 实验性功能测试

每个示例都包含完整的代码和注释，帮助你理解如何使用 VkCore 的各种功能。

## 常见问题

### Q: 如何处理窗口大小变化？

A: VkCore 会自动处理交换链重建，你只需要在窗口大小变化时调用：

```cpp
if (windowResized) {
    renderer.recreateSwapChain(newWidth, newHeight);
}
```

### Q: 如何添加更多纹理？

A: 简单地在 `draw()` 调用中指定不同的纹理路径：

```cpp
renderer.draw(vertices, indices, ubo, "diffuse.jpg");
renderer.draw(vertices, indices, ubo, "normal.jpg");
```

### Q: 如何实现透明渲染？

A: 修改着色器启用 alpha 混合：

```glsl
// 在片段着色器中
layout(location = 0) out vec4 outColor;
outColor = texture(texSampler, fragTexCoord); // 包含 alpha 通道
```

并启用 alpha 混合：

```cpp
// 在管线创建时启用混合
VkPipelineColorBlendAttachmentState colorBlendAttachment{};
colorBlendAttachment.blendEnable = VK_TRUE;
colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
// ...
```

这个指南涵盖了 VkCore 的主要使用方法。按照这些步骤，你可以构建各种 Vulkan 应用程序，从简单的 2D 游戏到复杂的 3D 场景。