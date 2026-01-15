#pragma once

/**
 * VkCore API 导出宏定义
 *
 * 此文件定义了 VkCore 库的导出/导入宏，用于 DLL 构建。
 * 当构建为 DLL 时，需要正确标记导出的符号。
 */

// 平台检测
#ifdef _WIN32
    #ifdef VKCORE_EXPORTS
        #define VKCORE_API __declspec(dllexport)
    #else
        #define VKCORE_API __declspec(dllimport)
    #endif
#else
    // 非 Windows 平台 (Linux, macOS)
    #define VKCORE_API __attribute__((visibility("default")))
#endif

// 导出控制宏
#define VKCORE_EXPORT VKCORE_API
#define VKCORE_IMPORT VKCORE_API

// 类导出宏
#define VKCORE_CLASS VKCORE_API

// 函数导出宏
#define VKCORE_FUNCTION VKCORE_API

// 模板实例化导出 (如果需要)
#define VKCORE_TEMPLATE VKCORE_API

// 版本信息
#define VKCORE_VERSION_MAJOR 1
#define VKCORE_VERSION_MINOR 0
#define VKCORE_VERSION_PATCH 0
#define VKCORE_VERSION_STRING "1.0.0"

// 编译时配置检查
#ifdef VKCORE_EXPORTS
    #define VKCORE_BUILDING_DLL
#endif

// 调试宏
#ifdef _DEBUG
    #define VKCORE_DEBUG
#endif

/**
 * VkCore API 命名空间
 *
 * 所有 VkCore 导出的类和函数都在此命名空间中
 */
#define VKCORE_BEGIN_NAMESPACE namespace VkCore {
#define VKCORE_END_NAMESPACE }

/**
 * 常用导出宏组合
 */

// 导出类声明
#define VKCORE_EXPORT_CLASS(className) \
    class VKCORE_CLASS className

// 导出结构体声明
#define VKCORE_EXPORT_STRUCT(structName) \
    struct VKCORE_CLASS structName

// 导出函数声明
#define VKCORE_EXPORT_FUNCTION(returnType, functionName) \
    VKCORE_FUNCTION returnType functionName

// 导出模板类
#define VKCORE_EXPORT_TEMPLATE_CLASS(templateParams, className) \
    template templateParams \
    class VKCORE_CLASS className

/**
 * 使用示例：
 *
 * // 在类声明前使用
 * VKCORE_EXPORT_CLASS(MyClass) {
 * public:
 *     VKCORE_FUNCTION void doSomething();
 * };
 *
 * // 在全局函数前使用
 * VKCORE_EXPORT_FUNCTION(void, globalFunction)(int param);
 *
 * // 在结构体前使用
 * VKCORE_EXPORT_STRUCT(MyStruct) {
 *     int value;
 * };
 */