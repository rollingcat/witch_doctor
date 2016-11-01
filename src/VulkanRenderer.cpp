/*
 * VulkanRenderer.cpp
 *
 *  Created on: Nov 1, 2016
 *      Author: hyowon
 */

#include "VulkanRenderer.h"

#include <assert.h>
#include <iostream>

void checkError(VkResult result) {
    if (result < 0) {
        switch (result) {
        case VK_ERROR_OUT_OF_HOST_MEMORY:
            std::cout << "VK_ERROR_OUT_OF_HOST_MEMORY" << std::endl;
            break;

        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
            std::cout << "VK_ERROR_OUT_OF_DEVICE_MEMORY" << std::endl;
            break;

        case VK_ERROR_INITIALIZATION_FAILED:
            std::cout << "VK_ERROR_INITIALIZATION_FAILED" << std::endl;
            break;

        case VK_ERROR_DEVICE_LOST:
            std::cout << "VK_ERROR_DEVICE_LOST" << std::endl;
            break;

        default:
            std::cout << "result: " << result << std::endl;
            break;
        }
    }
    assert(0 && "Vulkan Runtime Error");
}


VulkanRenderer::VulkanRenderer() {
    createInstance();
}


VulkanRenderer::~VulkanRenderer() {
    destroyInstance();
}


void VulkanRenderer::createInstance() {
    VkApplicationInfo app_info = {};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pNext = NULL;
    app_info.pApplicationName = "VooDoo";
    app_info.applicationVersion = 1;
    app_info.pEngineName = "Witch Doctor";
    app_info.engineVersion = 1;
    app_info.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo instance_create_info = {};
    instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_create_info.pNext = NULL;
    instance_create_info.flags = 0;
    instance_create_info.pApplicationInfo = &app_info;
    instance_create_info.enabledExtensionCount = 0;
    instance_create_info.ppEnabledExtensionNames = NULL;
    instance_create_info.enabledLayerCount = 0;
    instance_create_info.ppEnabledLayerNames = NULL;

    vkCreateInstance(&instance_create_info, nullptr, &mInstance);
}

void VulkanRenderer::destroyInstance() {
    vkDestroyInstance(mInstance, nullptr);
    mInstance = VK_NULL_HANDLE;
}


void VulkanRenderer::selectPhysicalDevice() {
    //Get the number of devices (GPUs) available.
    uint32_t gpu_count = 0;
    vkEnumeratePhysicalDevices(mInstance, &gpu_count, nullptr);

    std::vector<VkPhysicalDevice> gpuList(gpu_count);
    vkEnumeratePhysicalDevices(mInstance, &gpu_count, gpuList.data());
}























