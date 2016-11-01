/*
 * VulkanRenderer.h
 *
 *  Created on: Nov 1, 2016
 *      Author: hyowon
 */

#ifndef VULKANRENDERER_H_
#define VULKANRENDERER_H_

#include <vector>

#define VK_USE_PLATFORM_XCB_KHR
#include <vulkan/vulkan.h>

class VulkanRenderer
{
public:
    VulkanRenderer();
    ~VulkanRenderer();

private:
    void initLayersAndExtensions();
    void createInstance();
    void destroyInstance();

    void selectPhysicalDevice();
    void createLogicalDevice();
    void destroyLogicalDevice();

    void createCommandPool();
    void destroyCommandPool();
    void createCommandBuffer();

    VkInstance mInstance = VK_NULL_HANDLE;
    std::vector<const char*> mInstanceExtensions;
    VkPhysicalDevice mGpu = VK_NULL_HANDLE;
    VkDevice mDevice = VK_NULL_HANDLE;

    uint32_t mGraphcicQueueFamilyIndex;
    VkQueue mGraphicsQueue = VK_NULL_HANDLE;

    VkCommandPool mCommandPool = VK_NULL_HANDLE;
    VkCommandBuffer mCommandBuffer = VK_NULL_HANDLE;
};

#endif /* VULKANRENDERER_H_ */
