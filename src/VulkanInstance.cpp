/*
* VulkanInstance.cpp
*
*  Created on: Nov 17, 2016
*      Author: hyowon
*/

#include "VulkanInstance.h"

#include <iostream>
#include <vector>
#include <unordered_set>
#include <cassert>
#include <string.h>

#define DCHECK(condition) assert(condition);
#define DLOG(severity) std::cout

class VulkanInstance {
public:
  VulkanInstance() {}

  void Initialize() { mValid = InitializeVulkanInstance(); }

  bool InitializeVulkanInstance();

  bool mValid = false;
  VkInstance mInstance = VK_NULL_HANDLE;
#if DEBUG
  VkDebugReportCallbackEXT mErrorCallback = VK_NULL_HANDLE;
  VkDebugReportCallbackEXT mWarningCallback = VK_NULL_HANDLE;
#endif
};

static VulkanInstance* sVulkanInstance = nullptr;


VkBool32 VulkanErrorCallback(
    VkDebugReportFlagsEXT       flags,
    VkDebugReportObjectTypeEXT  objectType,
    uint64_t                    object,
    size_t                      location,
    int32_t                     messageCode,
    const char*                 pLayerPrefix,
    const char*                 pMessage,
    void*                       pUserData) {
  std::cout << pMessage;
  return VK_TRUE;
}

VkBool32 VulkanWarningCallback(
    VkDebugReportFlagsEXT       flags,
    VkDebugReportObjectTypeEXT  objectType,
    uint64_t                    object,
    size_t                      location,
    int32_t                     messageCode,
    const char*                 pLayerPrefix,
    const char*                 pMessage,
    void*                       pUserData) {
  std::cout << pMessage;
  return VK_TRUE;
}


bool VulkanInstance::InitializeVulkanInstance() {
  VkResult result = VK_SUCCESS;

  VkApplicationInfo app_info = {};
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pApplicationName = "VooDoo";
  app_info.apiVersion = VK_MAKE_VERSION(1, 0, 2);

  std::vector<const char*> enabled_ext_names;

  uint32_t num_instance_exts = 0;
  result = vkEnumerateInstanceExtensionProperties(nullptr, &num_instance_exts, nullptr);
  if (VK_SUCCESS != result) {
    std::cout << "vkEnumerateInstanceExtensionProperties(NULL) failed: " << result << std::endl;
    return false;
  }

  std::vector<VkExtensionProperties> instance_exts(num_instance_exts);
  result = vkEnumerateInstanceExtensionProperties(nullptr, &num_instance_exts,
                                                  instance_exts.data());
  if (VK_SUCCESS != result) {
    DLOG(ERROR) << "vkEnumerateInstanceExtensionProperties() failed: " << result << std::endl;
    return false;
  }

  bool debug_report_enabled = false;
  for (const VkExtensionProperties& ext_property : instance_exts) {
    if (strcmp(ext_property.extensionName,
               VK_EXT_DEBUG_REPORT_EXTENSION_NAME) == 0) {
      debug_report_enabled = true;
      enabled_ext_names.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    }
  }

  std::vector<const char*> enabled_layer_names;
#if DEBUG
  uint32_t num_instance_layers = 0;
  result = vkEnumerateInstanceLayerProperties(&num_instance_layers, nullptr);
  if (VK_SUCCESS != result) {
    std::cout << "vkEnumerateInstanceLayerProperties(NULL) failed: "
                << result << std::endl;
    return false;
  }

  std::vector<VkLayerProperties> instance_layers(num_instance_layers);
  result = vkEnumerateInstanceLayerProperties(&num_instance_layers,
                                              instance_layers.data());
  if (VK_SUCCESS != result) {
    std::cout << "vkEnumerateInstanceLayerProperties() failed: " << result << std::endl;
    return false;
  }

  std::unordered_set<std::string> desired_layers({
    "VK_LAYER_LUNARG_standard_validation",
  });

  for (const VkLayerProperties& layer_property : instance_layers) {
    if (desired_layers.find(layer_property.layerName) != desired_layers.end())
      enabled_layer_names.push_back(layer_property.layerName);
  }
#endif

  VkInstanceCreateInfo instance_create_info = {};
  instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instance_create_info.pApplicationInfo = &app_info;
  instance_create_info.enabledLayerCount = enabled_layer_names.size();
  instance_create_info.ppEnabledLayerNames = enabled_layer_names.data();
  instance_create_info.enabledExtensionCount = enabled_ext_names.size();
  instance_create_info.ppEnabledExtensionNames = enabled_ext_names.data();

  result = vkCreateInstance(&instance_create_info, nullptr, &mInstance);
  if (VK_SUCCESS != result) {
    std::cout << "vkCreateInstance() failed: " << result << std::endl;
    return false;
  }

#if DEBUG
  // Register our error logging function.
  if (debug_report_enabled) {
    PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT =
        reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>
            (vkGetInstanceProcAddr(mInstance,
                                   "vkCreateDebugReportCallbackEXT"));
    assert(vkCreateDebugReportCallbackEXT);

    VkDebugReportCallbackCreateInfoEXT cb_create_info = {};
    cb_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;

    cb_create_info.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT;
    cb_create_info.pfnCallback = &VulkanErrorCallback;
    result = vkCreateDebugReportCallbackEXT(mInstance, &cb_create_info,
                                            nullptr, &mErrorCallback);
    if (VK_SUCCESS != result) {
      std::cout << "vkCreateDebugReportCallbackEXT(ERROR) failed: "
                  << result << std::endl;
      return false;
    }

    cb_create_info.flags = VK_DEBUG_REPORT_WARNING_BIT_EXT |
                           VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
    cb_create_info.pfnCallback = &VulkanWarningCallback;
    result = vkCreateDebugReportCallbackEXT(mInstance, &cb_create_info,
                                            nullptr, &mWarningCallback);
    if (VK_SUCCESS != result) {
      std::cout << "vkCreateDebugReportCallbackEXT(WARN) failed: "
                  << result << std::endl;
      return false;
    }
  }
#endif
  return true;
}

bool InitializeVulkan() {
  DCHECK(!sVulkanInstance);
  sVulkanInstance = new VulkanInstance;
  sVulkanInstance->Initialize();
  return sVulkanInstance->mValid;
}

VkInstance GetVulkanInstance() {
  DCHECK(sVulkanInstance);
  DCHECK(sVulkanInstance->mValid);
  return sVulkanInstance->mInstance;
}


































