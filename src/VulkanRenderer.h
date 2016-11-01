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

    VkInstance mInstance = VK_NULL_HANDLE;
    std::vector<const char*> mInstanceExtensions;
};

#endif /* VULKANRENDERER_H_ */
