#if defined(_WIN32)
#	include "Render/Vulkan/Win32/ApiLoader.h"
#elif defined(__LINUX__)
#	include "Render/Vulkan/Linux/ApiLoader.h"
#elif defined(__ANDROID__)
#	include "Render/Vulkan/Android/ApiLoader.h"
#elif defined(__APPLE__)
#	include "Render/Vulkan/macOS/ApiLoader.h"
#endif

