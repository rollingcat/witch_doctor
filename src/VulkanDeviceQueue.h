/*
 * VulkanDeviceQueue.h
 *
 *  Created on: Nov 17, 2016
 *      Author: hyowon
 */

#ifndef VULKAN_DEVICE_QUEUE_H_
#define VULKAN_DEVICE_QUEUE_H_

#include <vulkan/vulkan.h>

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

  VkDevice GetVulkanDevice() const { return mDevice; }

private:
  VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
  VkDevice mDevice = VK_NULL_HANDLE;
  VkQueue mQueue = VK_NULL_HANDLE;
  uint32_t mQueueIndex = 0;
};



#endif /* VULKAN_DEVICE_QUEUE_H_ */
