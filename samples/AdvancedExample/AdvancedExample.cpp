// AdvancedExample.cpp : VkCore 高级使用示例
// 展示多个对象的渲染、纹理使用、动画等高级功能

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include <iostream>
#include <vector>
#include <array>
#include <chrono>
#include <memory>

// VkCore 头文件
#include "../../VkCore/Renderer.h"

// 简单的立方体顶点数据（位置 + 颜色 + 纹理坐标）
const std::vector<Vertex> cubeVertices = {
    // 前面
    {{-0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
    {{0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
    {{0.5f, 0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
    {{-0.5f, 0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},

    // 后面
    {{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
    {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
    {{0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
    {{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},

    // 左面
    {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
    {{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
    {{-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
    {{-0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},

    // 右面
    {{0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
    {{0.5f, -0.5f, 0.5f}, {1.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
    {{0.5f, 0.5f, 0.5f}, {1.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
    {{0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},

    // 上面
    {{-0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
    {{0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
    {{0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
    {{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

    // 下面
    {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
    {{0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
    {{0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
    {{-0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}
};

const std::vector<uint32_t> cubeIndices = {
    // 前面
    0, 1, 2, 2, 3, 0,
    // 后面
    4, 5, 6, 6, 7, 4,
    // 左面
    8, 9, 10, 10, 11, 8,
    // 右面
    12, 13, 14, 14, 15, 12,
    // 上面
    16, 17, 18, 18, 19, 16,
    // 下面
    20, 21, 22, 22, 23, 20
};

// 平面顶点数据（用于地面）
const std::vector<Vertex> planeVertices = {
    {{-2.0f, -1.0f, -2.0f}, {0.5f, 0.5f, 0.5f}, {0.0f, 0.0f}},
    {{2.0f, -1.0f, -2.0f}, {0.5f, 0.5f, 0.5f}, {2.0f, 0.0f}},
    {{2.0f, -1.0f, 2.0f}, {0.5f, 0.5f, 0.5f}, {2.0f, 2.0f}},
    {{-2.0f, -1.0f, 2.0f}, {0.5f, 0.5f, 0.5f}, {0.0f, 2.0f}}
};

const std::vector<uint32_t> planeIndices = {
    0, 1, 2, 2, 3, 0
};

// 渲染对象结构
struct RenderObject {
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
    std::string texturePath;
    const std::vector<Vertex>* vertices;
    const std::vector<uint32_t>* indices;

    glm::mat4 getModelMatrix() const {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, position);
        model = glm::rotate(model, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, scale);
        return model;
    }
};

int main() {
    // 初始化 GLFW
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    GLFWwindow* window = glfwCreateWindow(1200, 800, "VkCore - Advanced Example", nullptr, nullptr);

    // VkCore 配置
    VulkanConfig config;
    config.applicationName = "Advanced Example";
    config.windowWidth = 1200;
    config.windowHeight = 800;
    config.enableValidationLayers = true;

    try {
        // 创建渲染器
        Renderer renderer(config, window);
        renderer.initialize();

        std::cout << "VkCore 高级示例启动！" << std::endl;
        std::cout << "渲染多个旋转的立方体和地面..." << std::endl;

        // 定义渲染对象
        std::vector<RenderObject> objects = {
            // 中央大立方体
            {
                glm::vec3(0.0f, 0.0f, 0.0f),    // 位置
                glm::vec3(0.0f, 0.0f, 0.0f),    // 旋转
                glm::vec3(0.8f, 0.8f, 0.8f),    // 缩放
                "",                              // 无纹理，使用颜色
                &cubeVertices,
                &cubeIndices
            },
            // 左侧小立方体
            {
                glm::vec3(-1.5f, 0.5f, 0.0f),
                glm::vec3(0.0f, 0.0f, 0.0f),
                glm::vec3(0.3f, 0.3f, 0.3f),
                "",
                &cubeVertices,
                &cubeIndices
            },
            // 右侧小立方体
            {
                glm::vec3(1.5f, 0.5f, 0.0f),
                glm::vec3(0.0f, 0.0f, 0.0f),
                glm::vec3(0.3f, 0.3f, 0.3f),
                "",
                &cubeVertices,
                &cubeIndices
            },
            // 地面
            {
                glm::vec3(0.0f, 0.0f, 0.0f),
                glm::vec3(0.0f, 0.0f, 0.0f),
                glm::vec3(1.0f, 1.0f, 1.0f),
                "",
                &planeVertices,
                &planeIndices
            }
        };

        // 相机参数
        glm::vec3 cameraPos = glm::vec3(4.0f, 3.0f, 4.0f);
        glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

        // 主循环
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();

            // 更新时间
            static auto startTime = std::chrono::high_resolution_clock::now();
            auto currentTime = std::chrono::high_resolution_clock::now();
            float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

            // 旋转相机
            float radius = 5.0f;
            cameraPos.x = sin(time * 0.5f) * radius;
            cameraPos.z = cos(time * 0.5f) * radius;
            cameraPos.y = 2.0f + sin(time * 0.3f) * 0.5f;

            // 更新对象旋转
            objects[0].rotation = glm::vec3(time * 0.5f, time * 0.8f, time * 0.3f);  // 中央立方体
            objects[1].rotation = glm::vec3(0.0f, time * 1.2f, 0.0f);               // 左侧立方体
            objects[2].rotation = glm::vec3(time * 0.7f, 0.0f, time * 0.9f);       // 右侧立方体

            // 设置视图和投影矩阵
            glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, cameraUp);
            glm::mat4 proj = glm::perspective(glm::radians(60.0f), 1200.0f / 800.0f, 0.1f, 20.0f);
            proj[1][1] *= -1; // Vulkan Y轴翻转

            // 开始渲染
            renderer.beginFrame();

            // 渲染所有对象
            for (const auto& obj : objects) {
                UniformBufferObject ubo{};
                ubo.model = obj.getModelMatrix();
                ubo.view = view;
                ubo.proj = proj;

                renderer.draw(*obj.vertices, *obj.indices, ubo, obj.texturePath);
            }

            // 结束渲染
            renderer.endFrame();
        }

        renderer.waitIdle();

    } catch (const std::exception& e) {
        std::cerr << "VkCore 错误: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    std::cout << "高级示例正常退出。" << std::endl;
    return EXIT_SUCCESS;
}