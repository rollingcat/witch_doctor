
#include "VulkanRenderer.h"
#include "VulkanInstance.h"

#include <iostream>
#include <fstream>

static std::vector<char> ReadAllBytes(char const* filename);


VulkanRenderer::VulkanRenderer(GLFWwindow* window)
    : mWindow(window) {
}


bool VulkanRenderer::Init() {
  if (!createInstance()) {
    DLOG(ERROR) << "Failed to create Vulkan instance";
    return false;
  }

  createSurface();

  device_queue_.Initialize(VulkanDeviceQueue::GRAPHICS_QUEUE_FLAG |
                           VulkanDeviceQueue::PRESENTATION_SUPPORT_QUEUE_FLAG);

  selectPhysicalDevice();

  createLogicalDevice();

  createSwapchain();

  createImageViews();

  createRenderPass();

  createFrameBuffer();

  createCommandPool();

  createCommandBuffers();

  createGraphicsPipeline();

  recordCommandBuffer();

  createSemaphores();

  return true;
}


VulkanRenderer::~VulkanRenderer() {
    destroySemaphores();

    destroyGraphicsPipeline();

    destroyCommandPool();

    destroyFrameBuffer();

    destroyRenderPass();

    destroyImageViews();

    destroySwapchain();

    destroyLogicalDevice();

    destroySurface();

    destroyInstance();
}


void VulkanRenderer::render() {
    uint32_t image_idx;
    vkAcquireNextImageKHR(mDevice, mSwapchain, UINT64_MAX, mSemaphoreImageAvailable, VK_NULL_HANDLE, &image_idx);

    VkSemaphore wait_semaphores[] = { mSemaphoreImageAvailable };
    VkSemaphore signal_semaphores[] = { mSemaphoreRenderFinished };
    VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

    VkSubmitInfo submit_info {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_semaphores;
    submit_info.pWaitDstStageMask = wait_stages;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &mCommandBuffers[image_idx];
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_semaphores;

    vkQueueSubmit(mGraphicsQueue, 1, &submit_info, VK_NULL_HANDLE);

    VkSwapchainKHR swapchains[] = { mSwapchain };
    VkPresentInfoKHR present_info {};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = signal_semaphores;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swapchains;
    present_info.pImageIndices = &image_idx;
    present_info.pResults = nullptr;

    vkQueuePresentKHR(mPresentQueue, &present_info);
}


void VulkanRenderer::initExtensions()
{
    uint32_t count;
    const char** extensions = glfwGetRequiredInstanceExtensions(&count);

    for (uint32_t i = 0; i < count; ++i) {
        mInstanceExtensions.push_back(extensions[i]);
        LOG(INFO) << "glfw: " << extensions[i];
    }
}


bool VulkanRenderer::createInstance() {
  if (!InitializeVulkan())
    return false;

  mInstance = GetVulkanInstance();
  return true;
}

void VulkanRenderer::destroyInstance() {
  mInstance = VK_NULL_HANDLE;
}


void VulkanRenderer::selectPhysicalDevice() {
  mGpu = device_queue_.GetVulkanPhysicalDevice();
  mGraphicsQueueFamilyIndex = device_queue_.GetVulkanQueueIndex();
  mPresentQueueFamilyIndex = device_queue_.GetVulkanQueueIndex();
}

void VulkanRenderer::createLogicalDevice() {
  mDevice = device_queue_.GetVulkanDevice();
  mGraphicsQueue = device_queue_.GetVulkanQueue();
  mPresentQueue = device_queue_.GetVulkanQueue();
}


void VulkanRenderer::destroyLogicalDevice() {
  device_queue_.Destroy();
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


void VulkanRenderer::createCommandBuffers() {
    mCommandBuffers.resize(mSwapchainFramebuffers.size());

    VkCommandBufferAllocateInfo cmd_buffer_alloc_info{};
    cmd_buffer_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmd_buffer_alloc_info.pNext = NULL;
    cmd_buffer_alloc_info.commandPool = mCommandPool;
    cmd_buffer_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmd_buffer_alloc_info.commandBufferCount = (uint32_t)mCommandBuffers.size();

    vkAllocateCommandBuffers(mDevice, &cmd_buffer_alloc_info, mCommandBuffers.data());
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


void VulkanRenderer::createGraphicsPipeline() {
    VkShaderModule vert_shader_module;
    VkShaderModule frag_shader_module;

    auto vert_code = ReadAllBytes("./shader/vert.spv");
    auto frag_code = ReadAllBytes("./shader/frag.spv");

    createShaderModule(vert_code, vert_shader_module);
    createShaderModule(frag_code, frag_shader_module);

    VkPipelineShaderStageCreateInfo vert_stage_create_info{};
    vert_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vert_stage_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vert_stage_create_info.module = vert_shader_module;
    vert_stage_create_info.pName = "main";
    vert_stage_create_info.pSpecializationInfo = nullptr;

    VkPipelineShaderStageCreateInfo frag_stage_create_info{};
    frag_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    frag_stage_create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    frag_stage_create_info.module = frag_shader_module;
    frag_stage_create_info.pName = "main";
    frag_stage_create_info.pSpecializationInfo = nullptr;

    VkPipelineShaderStageCreateInfo shader_stages[] = { vert_stage_create_info, frag_stage_create_info };

    VkPipelineVertexInputStateCreateInfo vertex_input_info{};
    vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.vertexBindingDescriptionCount = 0;
    vertex_input_info.pVertexBindingDescriptions = nullptr;
    vertex_input_info.vertexAttributeDescriptionCount = 0;
    vertex_input_info.pVertexAttributeDescriptions = nullptr;

    VkPipelineInputAssemblyStateCreateInfo input_assembly{};
    input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport{ 0.0f, 0.0f, (float)mSwapchainExtent.width, (float)mSwapchainExtent.height };
    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = mSwapchainExtent;

    VkPipelineViewportStateCreateInfo viewport_state{};
    viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.viewportCount = 1;
    viewport_state.pViewports = &viewport;
    viewport_state.scissorCount = 1;
    viewport_state.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = 0.0f;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;
    multisampling.pSampleMask = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState color_blend_attachment {};
    color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                                            VK_COLOR_COMPONENT_G_BIT |
                                            VK_COLOR_COMPONENT_B_BIT |
                                            VK_COLOR_COMPONENT_A_BIT;
    color_blend_attachment.blendEnable = VK_FALSE;
    color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
    color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo color_blending {};
    color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blending.logicOpEnable = VK_FALSE;
    color_blending.logicOp = VK_LOGIC_OP_COPY;
    color_blending.attachmentCount = 1;
    color_blending.pAttachments = &color_blend_attachment;
    color_blending.blendConstants[0] = 0.0f;
    color_blending.blendConstants[1] = 0.0f;
    color_blending.blendConstants[2] = 0.0f;
    color_blending.blendConstants[3] = 0.0f;

    VkPipelineLayoutCreateInfo pipeline_layout_info;
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = 0;
    pipeline_layout_info.pSetLayouts = nullptr;
    pipeline_layout_info.pushConstantRangeCount = 0;
    pipeline_layout_info.pPushConstantRanges = 0;

    vkCreatePipelineLayout(mDevice, &pipeline_layout_info, nullptr, &mPipelineLayout);

    VkGraphicsPipelineCreateInfo pipeline_info {};
    pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.stageCount = 2;
    pipeline_info.pStages = shader_stages;
    pipeline_info.pVertexInputState = &vertex_input_info;
    pipeline_info.pInputAssemblyState = &input_assembly;
    pipeline_info.pViewportState = &viewport_state;
    pipeline_info.pRasterizationState = &rasterizer;
    pipeline_info.pMultisampleState = &multisampling;
    pipeline_info.pDepthStencilState = nullptr;
    pipeline_info.pColorBlendState = &color_blending;
    pipeline_info.pDynamicState = nullptr;
    pipeline_info.layout = mPipelineLayout;
    pipeline_info.renderPass = mRenderPass;
    pipeline_info.subpass = 0;
    pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
    pipeline_info.basePipelineIndex = -1;

    vkCreateGraphicsPipelines(mDevice, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &mPipeline);

    vkDestroyShaderModule(mDevice, frag_shader_module, nullptr);
    vkDestroyShaderModule(mDevice, vert_shader_module, nullptr);
}


void VulkanRenderer::destroyGraphicsPipeline() {
    vkDestroyPipeline(mDevice, mPipeline, nullptr);
    mPipeline = VK_NULL_HANDLE;
}


void VulkanRenderer::createShaderModule(const std::vector<char>& code, VkShaderModule& shaderModule) {
    VkShaderModuleCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = code.size();
    create_info.pCode = (uint32_t*)code.data();

    vkCreateShaderModule(mDevice, &create_info, nullptr, &shaderModule);
}


void VulkanRenderer::recordCommandBuffer() {
    for (size_t i = 0; i < mCommandBuffers.size(); ++i) {
        VkCommandBufferBeginInfo begin_info {};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        begin_info.pInheritanceInfo = nullptr;

        vkBeginCommandBuffer(mCommandBuffers[i], &begin_info);
        {
            VkRenderPassBeginInfo render_pass_begin_info {};
            render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            render_pass_begin_info.renderPass = mRenderPass;
            render_pass_begin_info.framebuffer = mSwapchainFramebuffers[i];

            render_pass_begin_info.renderArea.offset = { 0, 0 };
            render_pass_begin_info.renderArea.extent = mSwapchainExtent;

            VkClearValue clear_color = { 0.0f, 0.0f, 0.0f, 1.0f };
            render_pass_begin_info.clearValueCount = 1;
            render_pass_begin_info.pClearValues = &clear_color;

            vkCmdBeginRenderPass(mCommandBuffers[i], &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
            {
                vkCmdBindPipeline(mCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, mPipeline);
                vkCmdDraw(mCommandBuffers[i], 3, 1, 0, 0);
            }
            vkCmdEndRenderPass(mCommandBuffers[i]);
        }
        vkEndCommandBuffer(mCommandBuffers[i]);
    }
}


void VulkanRenderer::createSemaphores() {
    VkSemaphoreCreateInfo semaphore_info {};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    vkCreateSemaphore(mDevice, &semaphore_info, nullptr, &mSemaphoreImageAvailable);
    vkCreateSemaphore(mDevice, &semaphore_info, nullptr, &mSemaphoreRenderFinished);
}


void VulkanRenderer::destroySemaphores() {
    vkDestroySemaphore(mDevice, mSemaphoreImageAvailable, nullptr);
    vkDestroySemaphore(mDevice, mSemaphoreRenderFinished, nullptr);
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


static std::vector<char> ReadAllBytes(char const* filename)
{
    std::ifstream ifs(filename, std::ios::binary|std::ios::ate);
    if (!ifs.is_open()) {
        std::cout << "failed to load " << filename << std::endl;
        std::exit(-1);
    }
    std::ifstream::pos_type pos = ifs.tellg();

    std::vector<char> result(pos);

    ifs.seekg(0, std::ios::beg);
    ifs.read(&result[0], pos);

    return result;
}
