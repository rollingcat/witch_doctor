/*
 * VulkanRenderer.h
 *
 *  Created on: Nov 1, 2016
 *      Author: hyowon
 */

#ifndef VULKANRENDERER_H_
#define VULKANRENDERER_H_

#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class VulkanRenderer
{
public:
    VulkanRenderer(GLFWwindow*);
    ~VulkanRenderer();

private:
    void initExtensions();
    void createInstance();
    void destroyInstance();

    void selectPhysicalDevice();
    void createLogicalDevice();
    void destroyLogicalDevice();

    void createCommandPool();
    void destroyCommandPool();
    void createCommandBuffer();

    void createSurface();
    void destroySurface();

    GLFWwindow* mWindow = nullptr;

    VkInstance mInstance = VK_NULL_HANDLE;
    std::vector<const char*> mInstanceExtensions;
    VkPhysicalDevice mGpu = VK_NULL_HANDLE;
    VkDevice mDevice = VK_NULL_HANDLE;

    uint32_t mGraphicsQueueFamilyIndex = UINT32_MAX;
    uint32_t mPresentQueueFamilyIndex = UINT32_MAX;
    VkQueue mGraphicsQueue = VK_NULL_HANDLE;

    VkCommandPool mCommandPool = VK_NULL_HANDLE;
    VkCommandBuffer mCommandBuffer = VK_NULL_HANDLE;

    VkSurfaceKHR mSurface = VK_NULL_HANDLE;
};

#endif /* VULKANRENDERER_H_ */
