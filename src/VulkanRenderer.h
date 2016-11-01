/*
 * VulkanRenderer.h
 *
 *  Created on: Nov 1, 2016
 *      Author: hyowon
 */

#ifndef VULKANRENDERER_H_
#define VULKANRENDERER_H_

#include <vector>

#include <vulkan/vulkan.h>

class VulkanRenderer
{
public:
    VulkanRenderer();
    ~VulkanRenderer();

private:
    void createInstance();
    void destroyInstance();

    void selectPhysicalDevice();
    void createLogicalDevice();
    void destroyLogicalDevice();

    VkInstance mInstance = VK_NULL_HANDLE;
    VkPhysicalDevice mGpu = VK_NULL_HANDLE;
    VkDevice mDevice = VK_NULL_HANDLE;
    VkQueue mGraphicsQueue = VK_NULL_HANDLE;
    uint32_t mGraphcicQueueFamilyIndex;
};

#endif /* VULKANRENDERER_H_ */
