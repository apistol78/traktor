#if defined(_WIN32)
#	include "Render/Vulkan/Win32/ApiLoader.h"
#elif defined(__LINUX__) || defined(__RPI__)
#	include "Render/Vulkan/Linux/ApiLoader.h"
#elif defined(__ANDROID__)
#	include "Render/Vulkan/Android/ApiLoader.h"
#elif defined(__MAC__)
#	include "Render/Vulkan/macOS/ApiLoader.h"
#elif defined(__IOS__)
#	include "Render/Vulkan/iOS/ApiLoader.h"
#endif

