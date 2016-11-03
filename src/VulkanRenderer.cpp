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


VulkanRenderer::VulkanRenderer(GLFWwindow* window)
    : mWindow(window) {
    initExtensions();
    createInstance();

    createSurface();

    selectPhysicalDevice();
    createLogicalDevice();

    createCommandPool();

    createSwapchain();

    createImageViews();

    createRenderPass();

    createFrameBuffer();
}


VulkanRenderer::~VulkanRenderer() {
    destroyFrameBuffer();

    destroyRenderPass();

    destroyImageViews();

    destroySwapchain();

    destroyCommandPool();

    destroyLogicalDevice();

    destroySurface();

    destroyInstance();
}


void VulkanRenderer::initExtensions()
{
    uint32_t count;
    const char** extensions = glfwGetRequiredInstanceExtensions(&count);

    for (uint32_t i = 0; i < count; ++i) {
        mInstanceExtensions.push_back(extensions[i]);
    }
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
                mGraphicsQueueFamilyIndex = i;
            }

            VkBool32 present_support = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(gpu, i, mSurface, &present_support);
            if (present_support == VK_TRUE) {
                mPresentQueueFamilyIndex = i;
            }

            if (mGraphicsQueueFamilyIndex != UINT32_MAX && mPresentQueueFamilyIndex != UINT32_MAX) {
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
    queue_create_info.queueFamilyIndex = mGraphicsQueueFamilyIndex;
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

    vkGetDeviceQueue(mDevice, mGraphicsQueueFamilyIndex, 0, &mGraphicsQueue);
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
    cmd_pool_create_info.queueFamilyIndex = mGraphicsQueueFamilyIndex;
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


void VulkanRenderer::createSurface() {
    glfwCreateWindowSurface(mInstance, mWindow, NULL, &mSurface);
}


void VulkanRenderer::destroySurface() {
    vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
    mSurface = VK_NULL_HANDLE;
}


void VulkanRenderer::createSwapchain() {
    SwapchainInfo swapchain_info;
    swapchain_info.querySwapchainSupport(mGpu, mSurface);

    mSurfaceFormat = swapchain_info.chooseSwapchainFormat();
    mSwapchainExtent = swapchain_info.mCapabilities.currentExtent;

    VkSwapchainCreateInfoKHR swapchain_create_info;
    swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_create_info.surface = mSurface;
    swapchain_create_info.minImageCount = swapchain_info.mCapabilities.minImageCount;
    swapchain_create_info.imageFormat = mSurfaceFormat.format;
    swapchain_create_info.imageColorSpace = mSurfaceFormat.colorSpace;
    swapchain_create_info.imageExtent = mSwapchainExtent;
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_create_info.preTransform = swapchain_info.mCapabilities.currentTransform;
    swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchain_create_info.presentMode = swapchain_info.chooseSwapchainPresentMode();
    swapchain_create_info.clipped = VK_TRUE;
    swapchain_create_info.oldSwapchain = VK_NULL_HANDLE;

    vkCreateSwapchainKHR(mDevice, &swapchain_create_info, nullptr, &mSwapchain);

    uint32_t image_count = 0;
    vkGetSwapchainImagesKHR(mDevice, mSwapchain, &image_count, nullptr);
    mSwapchainImages.resize(image_count);
    vkGetSwapchainImagesKHR(mDevice, mSwapchain, &image_count, mSwapchainImages.data());
}


void VulkanRenderer::destroySwapchain() {
    vkDestroySwapchainKHR(mDevice, mSwapchain, nullptr);
    mSwapchain = VK_NULL_HANDLE;
}


void VulkanRenderer::createImageViews() {
    mSwapchainImageViews.resize(mSwapchainImages.size());

    for (uint32_t i = 0; i < mSwapchainImages.size(); i++) {
        VkImageViewCreateInfo imageview_create_info{};
        imageview_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageview_create_info.pNext = NULL;
        imageview_create_info.flags = 0;
        imageview_create_info.image = mSwapchainImages[i];
        imageview_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageview_create_info.format = mSurfaceFormat.format;
        imageview_create_info.components.r = VK_COMPONENT_SWIZZLE_R;
        imageview_create_info.components.g = VK_COMPONENT_SWIZZLE_G;
        imageview_create_info.components.b = VK_COMPONENT_SWIZZLE_B;
        imageview_create_info.components.a = VK_COMPONENT_SWIZZLE_A;
        imageview_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageview_create_info.subresourceRange.baseMipLevel = 0;
        imageview_create_info.subresourceRange.levelCount = 1;
        imageview_create_info.subresourceRange.baseArrayLayer = 0;
        imageview_create_info.subresourceRange.layerCount = 1;

        vkCreateImageView(mDevice, &imageview_create_info, nullptr, &mSwapchainImageViews[i]);
    }
}


void VulkanRenderer::destroyImageViews() {
    for (auto view : mSwapchainImageViews) {
        vkDestroyImageView(mDevice, view, nullptr);
    }
}


void VulkanRenderer::createRenderPass() {
    VkAttachmentDescription color_attachment{};
    color_attachment.format = mSurfaceFormat.format;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_attachment_ref{};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_ref;

    VkRenderPassCreateInfo render_pass_create_info{};
    render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_create_info.attachmentCount = 1;
    render_pass_create_info.pAttachments = &color_attachment;
    render_pass_create_info.subpassCount = 1;
    render_pass_create_info.pSubpasses = &subpass;
    render_pass_create_info.dependencyCount = 0;
    render_pass_create_info.pDependencies = nullptr;

    vkCreateRenderPass(mDevice, &render_pass_create_info, nullptr, &mRenderPass);
}


void VulkanRenderer::destroyRenderPass() {
    vkDestroyRenderPass(mDevice, mRenderPass, nullptr);
    mRenderPass = VK_NULL_HANDLE;
}


void VulkanRenderer::createFrameBuffer() {
    mSwapchainFramebuffers.resize(mSwapchainImageViews.size());

    for (size_t i = 0 ; i < mSwapchainImageViews.size(); ++i) {
        VkImageView attachments[] = { mSwapchainImageViews[i] };

        VkFramebufferCreateInfo frame_buffer_create_info{};
        frame_buffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        frame_buffer_create_info.pNext = NULL;
        frame_buffer_create_info.renderPass = mRenderPass;
        frame_buffer_create_info.attachmentCount = 1;
        frame_buffer_create_info.pAttachments = attachments;
        frame_buffer_create_info.width = mSwapchainExtent.width;
        frame_buffer_create_info.height = mSwapchainExtent.height;
        frame_buffer_create_info.layers = 1;

        vkCreateFramebuffer(mDevice, &frame_buffer_create_info, nullptr, &mSwapchainFramebuffers[i]);
    }
}


void VulkanRenderer::destroyFrameBuffer() {
    for (auto framebuffer : mSwapchainFramebuffers) {
        vkDestroyFramebuffer(mDevice, framebuffer, nullptr);
    }
}


void SwapchainInfo::querySwapchainSupport(VkPhysicalDevice gpu, VkSurfaceKHR surface) {
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &mCapabilities);

    uint32_t format_count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &format_count, nullptr);
    mSurfaceFormats.resize(format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &format_count, mSurfaceFormats.data());

    uint32_t present_mode_count = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &present_mode_count, nullptr);
    mPresentModes.resize(present_mode_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &present_mode_count, mPresentModes.data());
}


VkSurfaceFormatKHR SwapchainInfo::chooseSwapchainFormat() {
    for (const auto& surface_format : mSurfaceFormats) {
        if (surface_format.format == VK_FORMAT_R8G8B8A8_UNORM &&
            surface_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            return surface_format;
    }
    return mSurfaceFormats[0];
}


VkPresentModeKHR SwapchainInfo::chooseSwapchainPresentMode() {
    for (const auto& mode : mPresentModes) {
        if (mode == VK_PRESENT_MODE_FIFO_KHR)
            return mode;
    }
    return mPresentModes[0];
}

