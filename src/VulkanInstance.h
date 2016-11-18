
#ifndef VULKAN_INSTANCE_H_
#define VULKAN_INSTANCE_H_

#include <cassert>
#include <iostream>
#include <sstream>

#define WD_USE_GLFW 1

#include <vulkan/vulkan.h>

#if defined(NDEBUG)
#define DCHECK_IS_ON() 0
#else
#define DCHECK_IS_ON() 1
#endif

class LogMessageVoidify {
public:
  LogMessageVoidify() { }
  void operator&(std::ostream&) { }
};

#define LOG(severity) \
  LogMessage(__FILE__, __LINE__, #severity).stream()

#define EAT_STREAM_PARAMETERS \
  true ? (void) 0 : LogMessageVoidify() & LOG(FATAL)

#if DCHECK_IS_ON()
#define DCHECK(condition) assert(condition)
#define DCHECK_NE(val1, val2) assert(val1 != val2)
#define DCHECK_EQ(val1, val2) assert(val1 == val2)
#define DLOG(severity) LogMessage(__FILE__, __LINE__, #severity).stream()
#else
#define DCHECK_NE(val1, val2)
#define DCHECK_EQ(val1, val2)
#define DCHECK(condition)
#define DLOG(severity) EAT_STREAM_PARAMETERS
#endif

class LogMessage {
public:
  LogMessage(const char* file, int line, const char* severity) {
    stream_ << '[' << severity << "][" << file << "(" << line << ")] ";
  }
  ~LogMessage() {
    std::cout << stream_.str() << std::endl;
  }
  std::ostream& stream() { return stream_; }

private:
  std::ostringstream stream_;
};

template <typename T, size_t N> char (&ArraySizeHelper(T (&array)[N]))[N];
#define arraysize(array) (sizeof(ArraySizeHelper(array)))

bool InitializeVulkan();
bool VulkanSupported();

VkInstance GetVulkanInstance();

#endif /* VULKAN_INSTANCE_H_ */
