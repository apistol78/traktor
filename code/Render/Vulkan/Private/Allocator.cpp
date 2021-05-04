// Vulkan Memory Allocator implementation file.

#include "Render/Vulkan/Private/ApiHeader.h"
#include "Render/Vulkan/Private/ApiLoader.h"

#undef min
#undef max

#define VMA_IMPLEMENTATION
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#include <vk_mem_alloc.h>
