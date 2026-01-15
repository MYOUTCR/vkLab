// HelloVulkan.cpp : 使用 VkCore 的基础示例
// 展示如何使用 VkCore 进行基础的 Vulkan 初始化和渲染

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include <iostream>
#include <chrono>

// 包含 VkCore 头文件
// 注意：需要将 VkCore 项目添加到依赖中
#include "../../VkCore/Renderer.h"

// 定义顶点数据
const std::vector<Vertex> vertices = {
    {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // 左下 - 红色
    {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},  // 右下 - 绿色
    {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},   // 右上 - 蓝色
    {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}   // 左上 - 白色
};

const std::vector<uint32_t> indices = {
    0, 1, 2,  // 第一个三角形
    2, 3, 0   // 第二个三角形
};

int main() {
    // 初始化 GLFW
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // 不使用 OpenGL
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);    // 允许窗口调整大小

    // 创建窗口
    GLFWwindow* window = glfwCreateWindow(800, 600, "VkCore - Hello Vulkan", nullptr, nullptr);

    // 配置 VkCore
    VulkanConfig config;
    config.applicationName = "Hello Vulkan";
    config.windowWidth = 800;
    config.windowHeight = 600;
    config.enableValidationLayers = true;  // 开发时启用验证层

    try {
        // 创建 VkCore 渲染器
        Renderer renderer(config, window);

        // 初始化 Vulkan
        renderer.initialize();

        std::cout << "VkCore 初始化成功！" << std::endl;
        std::cout << "正在渲染旋转的四边形..." << std::endl;

        // 主渲染循环
        while (!glfwWindowShouldClose(window)) {
            // 处理窗口事件
            glfwPollEvents();

            // 计算旋转角度
            static auto startTime = std::chrono::high_resolution_clock::now();
            auto currentTime = std::chrono::high_resolution_clock::now();
            float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

            // 设置 Uniform 缓冲区数据
            UniformBufferObject ubo{};

            // 模型矩阵 - 绕 Z 轴旋转
            ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

            // 视图矩阵 - 相机位置
            ubo.view = glm::lookAt(
                glm::vec3(2.0f, 2.0f, 2.0f),    // 相机位置
                glm::vec3(0.0f, 0.0f, 0.0f),    // 目标位置
                glm::vec3(0.0f, 0.0f, 1.0f)     // 上方向
            );

            // 投影矩阵 - 透视投影
            ubo.proj = glm::perspective(
                glm::radians(45.0f),        // FOV
                800.0f / 600.0f,           // 宽高比
                0.1f,                      // 近裁剪面
                10.0f                      // 远裁剪面
            );
            ubo.proj[1][1] *= -1; // GLM 的 Y 轴与 Vulkan 相反，需要翻转

            // 渲染一帧
            renderer.beginFrame();
            renderer.draw(vertices, indices, ubo);  // 绘制四边形
            renderer.endFrame();
        }

        // 等待设备空闲，确保所有操作完成
        renderer.waitIdle();

    } catch (const std::exception& e) {
        std::cerr << "VkCore 错误: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    // 清理 GLFW
    glfwDestroyWindow(window);
    glfwTerminate();

    std::cout << "程序正常退出。" << std::endl;
    return EXIT_SUCCESS;
}