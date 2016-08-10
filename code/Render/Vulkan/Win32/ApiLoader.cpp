#include "Core/Log/Log.h"
#include "Render/Vulkan/Win32/ApiLoader.h"

namespace traktor
{
	namespace render
	{
		namespace
		{
		
HMODULE s_hVulkanModule = 0;

		}

PFN_vkCreateInstance vkCreateInstance = 0;
PFN_vkEnumerateInstanceLayerProperties vkEnumerateInstanceLayerProperties = 0;
PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = 0;

bool initializeVulkanApi()
{
	s_hVulkanModule = LoadLibrary(L"vulkan-1.dll");
	if (s_hVulkanModule == NULL)
	{
		log::error << L"Failed to load Vulkan library." << Endl;
		return false;
	}

	vkCreateInstance = (PFN_vkCreateInstance)GetProcAddress(s_hVulkanModule, "vkCreateInstance");
	if (vkCreateInstance == NULL)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkCreateInstance\"." << Endl;
		return false;
	}

	vkEnumerateInstanceLayerProperties = (PFN_vkEnumerateInstanceLayerProperties)GetProcAddress(s_hVulkanModule, "vkEnumerateInstanceLayerProperties");
	if (vkEnumerateInstanceLayerProperties == NULL)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkEnumerateInstanceLayerProperties\"." << Endl;
		return false;
	}

	vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)GetProcAddress(s_hVulkanModule, "vkGetInstanceProcAddr");
	if (vkGetInstanceProcAddr == NULL)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkGetInstanceProcAddr\"." << Endl;
		return false;
	}

	return true;
}

void finalizeVulkanApi()
{
}

	}
}
