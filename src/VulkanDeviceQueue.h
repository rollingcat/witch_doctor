/*
 * VulkanDeviceQueue.h
 *
 *  Created on: Nov 17, 2016
 *      Author: hyowon
 */

#ifndef VULKAN_DEVICE_QUEUE_H_
#define VULKAN_DEVICE_QUEUE_H_

#include <vulkan/vulkan.h>
#include "VulkanInstance.h"

class VulkanDeviceQueue
{
public:
  enum DeviceQueueOption {
    GRAPHICS_QUEUE_FLAG = 0x01,
    PRESENTATION_SUPPORT_QUEUE_FLAG = 0x02,
  };

  VulkanDeviceQueue();
  ~VulkanDeviceQueue();

  bool Initialize(uint32_t options);
  void Destroy();

  VkPhysicalDevice GetVulkanPhysicalDevice() const {
    DCHECK_NE(static_cast<VkPhysicalDevice>(VK_NULL_HANDLE),
              vk_physical_device_);
    return vk_physical_device_;
  }

  VkDevice GetVulkanDevice() const {
    DCHECK_NE(static_cast<VkDevice>(VK_NULL_HANDLE), vk_device_);
    return vk_device_;
  }

  VkQueue GetVulkanQueue() const {
    DCHECK_NE(static_cast<VkQueue>(VK_NULL_HANDLE), vk_queue_);
    return vk_queue_;
  }

  uint32_t GetVulkanQueueIndex() const { return vk_queue_index_; }
private:
  bool SelectPhysicalDevice(uint32_t options);

  VkPhysicalDevice vk_physical_device_ = VK_NULL_HANDLE;
  VkDevice vk_device_ = VK_NULL_HANDLE;
  VkQueue vk_queue_ = VK_NULL_HANDLE;
  uint32_t vk_queue_index_ = 0;
};

#endif /* VULKAN_DEVICE_QUEUE_H_ */
