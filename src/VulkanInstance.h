/*
 * VulkanInstance.h
 *
 *  Created on: Nov 17, 2016
 *      Author: hyowon
 */

#ifndef VULKAN_INSTANCE_H_
#define VULKAN_INSTANCE_H_

#include <vulkan/vulkan.h>

bool InitializeVulkan();

VkInstance GetVulkanInstance();

#endif /* VULKAN_INSTANCE_H_ */
