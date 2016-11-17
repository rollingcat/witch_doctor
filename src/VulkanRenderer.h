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

#include "VulkanDeviceQueue.h"

class VulkanRenderer
{
public:
    VulkanRenderer(GLFWwindow*);
    ~VulkanRenderer();

    void render();

private:
    void initExtensions();
    void createInstance();
    void destroyInstance();

    void createSurface();
    void destroySurface();

    void selectPhysicalDevice();
    void createLogicalDevice();
    void destroyLogicalDevice();

    void createCommandPool();
    void destroyCommandPool();
    void createCommandBuffers();

    void createSwapchain();
    void destroySwapchain();

    void createImageViews();
    void destroyImageViews();

    void createRenderPass();
    void destroyRenderPass();

    void createFrameBuffer();
    void destroyFrameBuffer();

    void createGraphicsPipeline();
    void destroyGraphicsPipeline();

    void createShaderModule(const std::vector<char>& code, VkShaderModule& shaderModule);

    void recordCommandBuffer();

    void createSemaphores();
    void destroySemaphores();

    GLFWwindow* mWindow = nullptr;

    VkInstance mInstance = VK_NULL_HANDLE;
    std::vector<const char*> mInstanceExtensions;
    VkPhysicalDevice mGpu = VK_NULL_HANDLE;
    VkDevice mDevice = VK_NULL_HANDLE;

    uint32_t mGraphicsQueueFamilyIndex = UINT32_MAX;
    uint32_t mPresentQueueFamilyIndex = UINT32_MAX;
    VkQueue mGraphicsQueue = VK_NULL_HANDLE;
    VkQueue mPresentQueue = VK_NULL_HANDLE;

    VkCommandPool mCommandPool = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> mCommandBuffers;

    VkSurfaceKHR mSurface = VK_NULL_HANDLE;
    VkSurfaceFormatKHR mSurfaceFormat;

    VkSwapchainKHR mSwapchain = VK_NULL_HANDLE;
    VkExtent2D mSwapchainExtent;
    std::vector<VkImage> mSwapchainImages;
    std::vector<VkImageView> mSwapchainImageViews;

    std::vector<VkFramebuffer> mSwapchainFramebuffers;

    VkRenderPass mRenderPass = VK_NULL_HANDLE;

    VkPipelineLayout mPipelineLayout = VK_NULL_HANDLE;
    VkPipeline mPipeline = VK_NULL_HANDLE;

    VkSemaphore mSemaphoreImageAvailable = VK_NULL_HANDLE;
    VkSemaphore mSemaphoreRenderFinished = VK_NULL_HANDLE;

    VulkanDeviceQueue device_queue_;
};

class SwapchainInfo {
public:
    void querySwapchainSupport(VkPhysicalDevice, VkSurfaceKHR);

    VkSurfaceFormatKHR chooseSwapchainFormat();
    VkPresentModeKHR chooseSwapchainPresentMode();

    VkSurfaceCapabilitiesKHR mCapabilities;
    std::vector<VkSurfaceFormatKHR> mSurfaceFormats;
    std::vector<VkPresentModeKHR> mPresentModes;
};

#endif /* VULKANRENDERER_H_ */
