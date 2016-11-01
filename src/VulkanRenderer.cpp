/*
 * VulkanRenderer.cpp
 *
 *  Created on: Nov 1, 2016
 *      Author: hyowon
 */

#include "VulkanRenderer.h"

#include <assert.h>
#include <iostream>
#include <vector>

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
    initLayersAndExtensions();
    createInstance();

    selectPhysicalDevice();
    createLogicalDevice();

    createCommandPool();
}


VulkanRenderer::~VulkanRenderer() {
    destroyCommandPool();

    destroyLogicalDevice();

    destroyInstance();
}


void VulkanRenderer::initLayersAndExtensions()
{
    mInstanceExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
    mInstanceExtensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
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
    instance_create_info.enabledExtensionCount = mInstanceExtensions.size();
    instance_create_info.ppEnabledExtensionNames = mInstanceExtensions.empty()
                                                   ? nullptr
                                                   : mInstanceExtensions.data();
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

    bool found = false;
    for (const auto& gpu : gpuList) {
        uint32_t property_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(gpu, &property_count, nullptr);
        std::vector<VkQueueFamilyProperties> properties(property_count);
        vkGetPhysicalDeviceQueueFamilyProperties(gpu, &property_count, properties.data());

        for (uint32_t i = 0; i < property_count; ++i) {
            if (properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                mGraphcicQueueFamilyIndex = i;
                found = true;
                break;
            }
        }

        if (found) {
            mGpu = gpu;
            break;
        }
    }

    if (mGpu == VK_NULL_HANDLE) {
        std::cout << "Failed to find a suitable GPU!" << std::endl;
        std::exit(-1);
    }
}

void VulkanRenderer::createLogicalDevice() {
    float queue_priorities[1] = {0.0};
    VkDeviceQueueCreateInfo queue_create_info{};
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.queueFamilyIndex = mGraphcicQueueFamilyIndex;
    queue_create_info.queueCount = 1;
    queue_create_info.pQueuePriorities = queue_priorities;

    VkDeviceCreateInfo device_create_info{};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.queueCreateInfoCount = 1;
    device_create_info.pQueueCreateInfos = &queue_create_info;
    device_create_info.enabledExtensionCount = 0;
    device_create_info.ppEnabledExtensionNames = nullptr;
    device_create_info.enabledLayerCount = 0;
    device_create_info.ppEnabledLayerNames = nullptr;
    device_create_info.pEnabledFeatures = nullptr;

    vkCreateDevice(mGpu, &device_create_info, nullptr, &mDevice);

    vkGetDeviceQueue(mDevice, mGraphcicQueueFamilyIndex, 0, &mGraphicsQueue);
}


void VulkanRenderer::destroyLogicalDevice() {
    vkDestroyDevice(mDevice, nullptr);
    mDevice = VK_NULL_HANDLE;
}


void VulkanRenderer::createCommandPool()
{
    VkCommandPoolCreateInfo cmd_pool_create_info{};
    cmd_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmd_pool_create_info.pNext = NULL;
    cmd_pool_create_info.queueFamilyIndex = mGraphcicQueueFamilyIndex;
    cmd_pool_create_info.flags = 0;

    vkCreateCommandPool(mDevice, &cmd_pool_create_info, nullptr, &mCommandPool);
}


void VulkanRenderer::destroyCommandPool() {
    vkDestroyCommandPool(mDevice, mCommandPool, nullptr);
}


void VulkanRenderer::createCommandBuffer() {
    VkCommandBufferAllocateInfo cmd_buffer_alloc_info{};
    cmd_buffer_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmd_buffer_alloc_info.pNext = NULL;
    cmd_buffer_alloc_info.commandPool = mCommandPool;
    cmd_buffer_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmd_buffer_alloc_info.commandBufferCount = 1;

    vkAllocateCommandBuffers(mDevice, &cmd_buffer_alloc_info, &mCommandBuffer);
}

















