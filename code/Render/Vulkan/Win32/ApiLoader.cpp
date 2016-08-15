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
PFN_vkEnumerateInstanceExtensionProperties vkEnumerateInstanceExtensionProperties = 0;
PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices = 0;
PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties = 0;
PFN_vkGetPhysicalDeviceQueueFamilyProperties vkGetPhysicalDeviceQueueFamilyProperties = 0;
PFN_vkGetPhysicalDeviceMemoryProperties vkGetPhysicalDeviceMemoryProperties = 0;
PFN_vkCreateDevice vkCreateDevice = 0;
PFN_vkGetDeviceQueue vkGetDeviceQueue = 0;
PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = 0;
PFN_vkCreateCommandPool vkCreateCommandPool = 0;
PFN_vkAllocateCommandBuffers vkAllocateCommandBuffers = 0;
PFN_vkCreateShaderModule vkCreateShaderModule = 0;

PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR = 0;
PFN_vkGetPhysicalDeviceSurfaceSupportKHR vkGetPhysicalDeviceSurfaceSupportKHR = 0;

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

	vkEnumerateInstanceExtensionProperties = (PFN_vkEnumerateInstanceExtensionProperties)GetProcAddress(s_hVulkanModule, "vkEnumerateInstanceExtensionProperties");
	if (vkEnumerateInstanceExtensionProperties == NULL)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkEnumerateInstanceExtensionProperties\"." << Endl;
		return false;
	}

	vkEnumeratePhysicalDevices = (PFN_vkEnumeratePhysicalDevices)GetProcAddress(s_hVulkanModule, "vkEnumeratePhysicalDevices");
	if (vkEnumeratePhysicalDevices == NULL)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkEnumeratePhysicalDevices\"." << Endl;
		return false;
	}

	vkGetPhysicalDeviceProperties = (PFN_vkGetPhysicalDeviceProperties)GetProcAddress(s_hVulkanModule, "vkGetPhysicalDeviceProperties");
	if (vkGetPhysicalDeviceProperties == NULL)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkGetPhysicalDeviceProperties\"." << Endl;
		return false;
	}

	vkGetPhysicalDeviceQueueFamilyProperties = (PFN_vkGetPhysicalDeviceQueueFamilyProperties)GetProcAddress(s_hVulkanModule, "vkGetPhysicalDeviceQueueFamilyProperties");
	if (vkGetPhysicalDeviceQueueFamilyProperties == NULL)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkGetPhysicalDeviceQueueFamilyProperties\"." << Endl;
		return false;
	}

	vkGetPhysicalDeviceMemoryProperties = (PFN_vkGetPhysicalDeviceMemoryProperties)GetProcAddress(s_hVulkanModule, "vkGetPhysicalDeviceMemoryProperties");
	if (vkGetPhysicalDeviceMemoryProperties == NULL)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkGetPhysicalDeviceMemoryProperties\"." << Endl;
		return false;
	}

	vkCreateDevice = (PFN_vkCreateDevice)GetProcAddress(s_hVulkanModule, "vkCreateDevice");
	if (vkCreateDevice == NULL)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkCreateDevice\"." << Endl;
		return false;
	}

	vkGetDeviceQueue = (PFN_vkGetDeviceQueue)GetProcAddress(s_hVulkanModule, "vkGetDeviceQueue");
	if (vkGetDeviceQueue == NULL)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkGetDeviceQueue\"." << Endl;
		return false;
	}

	vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)GetProcAddress(s_hVulkanModule, "vkGetInstanceProcAddr");
	if (vkGetInstanceProcAddr == NULL)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkGetInstanceProcAddr\"." << Endl;
		return false;
	}

	vkCreateCommandPool = (PFN_vkCreateCommandPool)GetProcAddress(s_hVulkanModule, "vkCreateCommandPool");
	if (vkCreateCommandPool == NULL)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkCreateCommandPool\"." << Endl;
		return false;
	}

	vkAllocateCommandBuffers = (PFN_vkAllocateCommandBuffers)GetProcAddress(s_hVulkanModule, "vkAllocateCommandBuffers");
	if (vkAllocateCommandBuffers == NULL)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkAllocateCommandBuffers\"." << Endl;
		return false;
	}

	vkCreateShaderModule = (PFN_vkCreateShaderModule)GetProcAddress(s_hVulkanModule, "vkCreateShaderModule");
	if (vkCreateShaderModule == NULL)
	{
		log::error << L"Failed to resolve Vulkan entry point \"vkCreateShaderModule\"." << Endl;
		return false;
	}

	return true;
}

bool initializeVulkanExtensions(VkInstance instance)
{
	*(void**)&vkCreateWin32SurfaceKHR = vkGetInstanceProcAddr(instance, "vkCreateWin32SurfaceKHR");
	if (vkCreateWin32SurfaceKHR == NULL)
	{
		log::error << L"Failed to resolve Vulkan extension entry point \"vkCreateWin32SurfaceKHR\"." << Endl;
		return false;
	}

	*(void**)&vkGetPhysicalDeviceSurfaceSupportKHR = vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfaceSupportKHR");
	if (vkGetPhysicalDeviceSurfaceSupportKHR == NULL)
	{
		log::error << L"Failed to resolve Vulkan extension entry point \"vkGetPhysicalDeviceSurfaceSupportKHR\"." << Endl;
		return false;
	}

	return true;
}

void finalizeVulkanApi()
{
}

	}
}
