# VkCore 导出宏使用指南

## 概述

VkCore 定义了一套完整的导出宏系统，支持静态库和动态库 (DLL) 两种构建方式。通过导出宏，可以精确控制哪些 API 是公开的，哪些是内部使用的。

## 宏定义

### 基础宏

```cpp
// VkCoreAPI.h 中的核心定义

// 平台检测和导出控制
#ifdef _WIN32
    #ifdef VKCORE_EXPORTS
        #define VKCORE_API __declspec(dllexport)
    #else
        #define VKCORE_API __declspec(dllimport)
    #endif
#else
    #define VKCORE_API __attribute__((visibility("default")))
#endif

// 便捷别名
#define VKCORE_EXPORT VKCORE_API
#define VKCORE_CLASS VKCORE_API
#define VKCORE_FUNCTION VKCORE_API
```

### 复合宏

```cpp
// 类导出
#define VKCORE_EXPORT_CLASS(className) class VKCORE_CLASS className

// 结构体导出
#define VKCORE_EXPORT_STRUCT(structName) struct VKCORE_CLASS structName

// 函数导出
#define VKCORE_EXPORT_FUNCTION(returnType, functionName) VKCORE_FUNCTION returnType functionName

// 模板导出
#define VKCORE_EXPORT_TEMPLATE_CLASS(templateParams, className) \
    template templateParams class VKCORE_CLASS className
```

## 使用方法

### 1. 类声明

```cpp
// VulkanInstance.h
#pragma once
#include "VkCoreAPI.h"

VKCORE_EXPORT_CLASS(VulkanInstance) {
public:
    VKCORE_FUNCTION VulkanInstance(const VulkanConfig& config);
    VKCORE_FUNCTION ~VulkanInstance();

    VKCORE_FUNCTION VkInstance getInstance() const;
    VKCORE_FUNCTION void createSurface(GLFWwindow* window);

private:
    VkInstance instance_;
    // ... 私有成员不需要导出
};
```

### 2. 结构体声明

```cpp
// Types.h
#pragma once
#include "VkCoreAPI.h"

VKCORE_EXPORT_STRUCT(VulkanConfig) {
    std::string applicationName = "VkCore Application";
    uint32_t applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    uint32_t windowWidth = 800;
    uint32_t windowHeight = 600;
    bool enableValidationLayers = true;
};

VKCORE_EXPORT_STRUCT(Vertex) {
    glm::vec3 position;
    glm::vec3 color;
    glm::vec2 texCoord;

    static VkVertexInputBindingDescription getBindingDescription();
    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions();
};
```

### 3. 全局函数

```cpp
// Utils.h
#pragma once
#include "VkCoreAPI.h"

VKCORE_EXPORT_FUNCTION(std::vector<char>, readFile)(const std::string& filename);
VKCORE_EXPORT_FUNCTION(void, checkVkResult)(VkResult result, const std::string& message);
```

### 4. 模板类

```cpp
// 针对特定实例化的模板
template<typename T>
VKCORE_EXPORT_TEMPLATE_CLASS(, ResourcePtr<T>) {
    // ...
};

// 或者使用显式实例化
template class VKCORE_CLASS ResourceManager;
```

## 构建配置

### 静态库构建

- **默认配置**: 不需要特殊设置
- **宏状态**: `VKCORE_API` 被定义为空
- **使用**: 直接链接 `.lib` 文件

### DLL 构建

#### VkCore 库构建
```cpp
// 在 VkCoreDLL 项目中
#define VKCORE_EXPORTS  // 这会使 VKCORE_API = __declspec(dllexport)
```

#### 应用程序使用 DLL
```cpp
// 在使用 VkCore.dll 的项目中
// 不定义 VKCORE_EXPORTS，所以 VKCORE_API = __declspec(dllimport)
// 自动从 DLL 导入符号
```

## 项目配置

### Visual Studio 配置

1. **静态库项目** (`VkCore.vcxproj`):
   - 配置类型: 静态库 (.lib)
   - 不定义任何导出宏

2. **DLL 项目** (`VkCoreDLL.vcxproj`):
   - 配置类型: 动态库 (.dll)
   - 预处理器定义: `VKCORE_EXPORTS`

3. **使用 DLL 的项目**:
   - 链接: `VkCore.lib` (导入库)
   - 运行时: 需要 `VkCore.dll` 在 PATH 中

### CMake 配置 (可选)

```cmake
# VkCore 静态库
add_library(VkCore STATIC ${SOURCES})

# VkCore DLL
add_library(VkCoreDLL SHARED ${SOURCES})
target_compile_definitions(VkCoreDLL PRIVATE VKCORE_EXPORTS)

# 使用 VkCore 的应用
add_executable(MyApp main.cpp)
target_link_libraries(MyApp VkCore)  # 或 VkCoreDLL
```

## 最佳实践

### 1. 导出策略

- **导出**: 所有公共 API 类、函数、结构体
- **不导出**: 私有实现类、内部工具函数
- **谨慎导出**: 模板类，避免过度导出

### 2. 命名空间

```cpp
// 推荐：使用命名空间包装
VKCORE_BEGIN_NAMESPACE

VKCORE_EXPORT_CLASS(Renderer) {
    // ...
};

VKCORE_END_NAMESPACE
```

### 3. 版本控制

```cpp
// 版本宏
#define VKCORE_VERSION_MAJOR 1
#define VKCORE_VERSION_MINOR 0
#define VKCORE_VERSION_PATCH 0

// 条件导出（未来版本使用）
#if VKCORE_VERSION >= VK_MAKE_VERSION(2, 0, 0)
VKCORE_EXPORT_FUNCTION(void, newFeature)();
#endif
```

## 常见问题

### Q: 链接错误 "unresolved external symbol"

**A**: 检查预处理器宏定义：
- DLL 构建时必须定义 `VKCORE_EXPORTS`
- 使用 DLL 时不能定义 `VKCORE_EXPORTS`

### Q: "dllimport not allowed on data"

**A**: 静态成员变量不能用 dllimport，改为：
```cpp
class VKCORE_CLASS MyClass {
public:
    static const int VALUE;  // 声明，不用导出宏
};

// 实现文件中定义
const int MyClass::VALUE = 42;
```

### Q: Linux/MacOS 支持

**A**: 使用 `__attribute__((visibility("default")))` 而不是 `__declspec`。

### Q: 模板导出问题

**A**: 在实现文件中显式实例化：
```cpp
// .cpp 文件中
template class VKCORE_CLASS MyTemplate<int>;
template class VKCORE_CLASS MyTemplate<float>;
```

## 示例项目

### 静态库使用
```cpp
#include "Renderer.h"  // 自动处理导出宏

int main() {
    VulkanConfig config;
    Renderer renderer(config, window);  // 静态链接
}
```

### DLL 动态加载
```cpp
#include <windows.h>
#include "Renderer.h"

int main() {
    // 运行时加载 DLL
    HMODULE dll = LoadLibrary("VkCore.dll");
    if (dll) {
        // 使用函数指针调用
        typedef Renderer* (*CreateRendererFunc)(const VulkanConfig&, GLFWwindow*);
        auto createRenderer = (CreateRendererFunc)GetProcAddress(dll, "CreateRenderer");

        if (createRenderer) {
            Renderer* renderer = createRenderer(config, window);
            // 使用 renderer...
        }
        FreeLibrary(dll);
    }
}
```

## 总结

VkCore 的导出宏系统提供了：

- ✅ **跨平台兼容**: Windows DLL + Unix 共享库
- ✅ **灵活构建**: 静态库和动态库皆可
- ✅ **精确控制**: 选择性导出公共 API
- ✅ **易于使用**: 统一的宏接口
- ✅ **版本安全**: 支持 API 版本控制

通过这套导出宏系统，VkCore 可以在不同的部署场景中灵活使用，既可以作为静态库直接链接，也可以作为 DLL 动态加载。