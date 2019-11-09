#if defined(__ANDROID__)
#	include <android_native_app_glue.h>
#endif
#include <cstring>
#include "Core/Log/Log.h"
#include "Core/Misc/Adler32.h"
#include "Core/Misc/Align.h"
#include "Core/Misc/AutoPtr.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/TString.h"
#include "Render/VertexElement.h"
#include "Render/Vulkan/ApiLoader.h"
#include "Render/Vulkan/CubeTextureVk.h"
#include "Render/Vulkan/IndexBufferVk.h"
#include "Render/Vulkan/ProgramVk.h"
#include "Render/Vulkan/ProgramResourceVk.h"
#include "Render/Vulkan/RenderSystemVk.h"
#include "Render/Vulkan/RenderTargetDepthVk.h"
#include "Render/Vulkan/RenderTargetSetVk.h"
#include "Render/Vulkan/RenderTargetVk.h"
#include "Render/Vulkan/RenderViewVk.h"
#include "Render/Vulkan/SimpleTextureVk.h"
#include "Render/Vulkan/StructBufferVk.h"
#include "Render/Vulkan/TimeQueryVk.h"
#include "Render/Vulkan/UtilitiesVk.h"
#include "Render/Vulkan/VertexAttributesVk.h"
#include "Render/Vulkan/VertexBufferDynamicVk.h"
#include "Render/Vulkan/VolumeTextureVk.h"
#include "Render/Vulkan/Editor/Glsl/GlslType.h"

#if defined(_WIN32)
#	include "Render/Vulkan/Win32/Window.h"
#elif defined(__LINUX__)
#	include "Render/Vulkan/Linux/Window.h"
#elif defined(__ANDROID__)
#	include "Core/System/Android/DelegateInstance.h"
#elif defined(__IOS__)
#	include "Render/Vulkan/iOS/Utilities.h"
#endif

#define T_ENABLE_VALIDATION 0

namespace traktor
{
	namespace render
	{
		namespace
		{

const char* c_validationLayerNames[] = { "VK_LAYER_KHRONOS_validation", nullptr };
#if defined(_WIN32)
const char* c_extensions[] = { "VK_KHR_surface", "VK_KHR_win32_surface", "VK_EXT_debug_utils" };
#elif defined(__LINUX__)
const char* c_extensions[] = { "VK_KHR_surface", "VK_KHR_xlib_surface", "VK_EXT_debug_utils" };
#elif defined(__ANDROID__)
const char* c_extensions[] = { "VK_KHR_surface", "VK_KHR_android_surface" };
#elif defined(__MAC__)
const char* c_extensions[] = { "VK_KHR_surface", "VK_MVK_macos_surface", "VK_EXT_debug_utils" };
#else
const char* c_extensions[] = { "VK_KHR_surface", "VK_EXT_debug_utils" };
#endif
const char* c_deviceExtensions[] = { "VK_KHR_swapchain" };

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData
)
{
	if (pCallbackData && pCallbackData->pMessage)
		log::info << mbstows(pCallbackData->pMessage) << Endl;
	return VK_FALSE;
}
		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.RenderSystemVk", 0, RenderSystemVk, IRenderSystem)

RenderSystemVk::RenderSystemVk()
#if defined(__LINUX__)
:	m_display(nullptr)
,	m_instance(0)
#else
:	m_instance(0)
#endif
,	m_physicalDevice(0)
,	m_logicalDevice(0)
,	m_debugMessenger(0)
,	m_graphicsQueueIndex(~0)
,	m_computeQueueIndex(~0)
,	m_graphicsQueue(0)
,	m_computeQueue(0)
,	m_graphicsCommandPool(0)
,	m_allocator(0)
{
#if defined(__ANDROID__)
	m_screenWidth = 0;
	m_screenHeight = 0;
#endif
}

bool RenderSystemVk::create(const RenderSystemDesc& desc)
{
	VkResult result;

#if defined(__LINUX__)
	if ((m_display = XOpenDisplay(0)) == nullptr)
	{
		log::error << L"Unable to create Vulkan renderer; Failed to open X display" << Endl;
		return false;
	}
#elif defined(__ANDROID__)
	auto app = desc.sysapp.instance->getApplication();
	m_screenWidth = ANativeWindow_getWidth(app->window);
	m_screenHeight = ANativeWindow_getHeight(app->window);
#endif

	if (!initializeVulkanApi())
	{
		log::error << L"Unable to create Vulkan renderer; Failed to initialize Vulkan API." << Endl;
		return false;
	}

	// Check for available layers.
	uint32_t layerCount = 0;
	vkEnumerateInstanceLayerProperties(&layerCount, 0);

	AutoArrayPtr< VkLayerProperties > layersAvailable(new VkLayerProperties[layerCount]);
	vkEnumerateInstanceLayerProperties(&layerCount, layersAvailable.ptr());

	AlignedVector< const char* > validationLayers;
#if T_ENABLE_VALIDATION
	for (uint32_t i = 0; i < layerCount; ++i)
	{
		bool found = false;
		for (uint32_t j = 0; c_validationLayerNames[j] != nullptr; ++j)
		{
			if (strcmp(layersAvailable[i].layerName, c_validationLayerNames[j]) == 0)
				found = true;
		}
		if (found)
			validationLayers.push_back(strdup(layersAvailable[i].layerName));
	}
#endif

	// Create Vulkan instance.
	VkApplicationInfo ai = {};
	ai.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	ai.pNext = nullptr;
	ai.pApplicationName = "Traktor";
	ai.pEngineName = "Traktor";
	ai.engineVersion = 1;
	ai.apiVersion = VK_MAKE_VERSION(1, 0, 0);

	VkInstanceCreateInfo ii = {};
	ii.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	ii.pApplicationInfo = &ai;
	ii.enabledLayerCount = (uint32_t)validationLayers.size();
	ii.ppEnabledLayerNames = validationLayers.c_ptr();
	ii.enabledExtensionCount = sizeof_array(c_extensions);
	ii.ppEnabledExtensionNames = c_extensions;

	if ((result = vkCreateInstance(&ii, 0, &m_instance)) != VK_SUCCESS)
	{
		log::error << L"Failed to create Vulkan instance (" << getHumanResult(result) << L")." << Endl;
		return false;
	}

	// Load Vulkan extensions.
	if (!initializeVulkanExtensions(m_instance))
	{
		log::error << L"Failed to create Vulkan; failed to load extensions." << Endl;
		return false;
	}

#if !defined(__ANDROID__)
#	if T_ENABLE_VALIDATION
	// Setup debug port callback.
	VkDebugUtilsMessengerCreateInfoEXT dumci = {};
	dumci.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	dumci.messageSeverity = /*VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | */VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	dumci.messageType = /*VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | */VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT /*| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT*/;
	dumci.pfnUserCallback = debugCallback;

	if ((result = vkCreateDebugUtilsMessengerEXT(m_instance, &dumci, nullptr, &m_debugMessenger)) != VK_SUCCESS)
	{
		log::error << L"Failed to create Vulkan; failed to set debug report callback." << Endl;
		return false;
	}
#	endif
#endif

	// Select physical device.
	uint32_t physicalDeviceCount = 0;
	vkEnumeratePhysicalDevices(m_instance, &physicalDeviceCount, 0);
	if (physicalDeviceCount == 0)
	{
		log::error << L"Failed to create Vulkan; no physical devices." << Endl;
		return false;
	}

	// For now select first reported device; \tbd need to revisit this.
	AutoArrayPtr< VkPhysicalDevice > physicalDevices(new VkPhysicalDevice[physicalDeviceCount]);
	vkEnumeratePhysicalDevices(m_instance, &physicalDeviceCount, physicalDevices.ptr());
	m_physicalDevice = physicalDevices[0];

	// Get physical device graphics queue.
	m_graphicsQueueIndex = ~0;
	m_computeQueueIndex = ~0;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, 0);

	AutoArrayPtr< VkQueueFamilyProperties > queueFamilyProperties(new VkQueueFamilyProperties[queueFamilyCount]);
	vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, queueFamilyProperties.ptr());

	for (uint32_t i = 0; i < queueFamilyCount; ++i)
	{
		if (m_graphicsQueueIndex == ~0 && queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			m_graphicsQueueIndex = i;
		if (m_computeQueueIndex == ~0 && queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
			m_computeQueueIndex = i;
	}
	if (m_graphicsQueueIndex == ~0)
	{
		log::error << L"Failed to create Vulkan; no suitable graphics queue found." << Endl;
		return false;
	}
	if (m_computeQueueIndex == ~0)
	{
		log::error << L"Failed to create Vulkan; no suitable compute queue found." << Endl;
		return false;
	}

	// Create logical device.
    VkDeviceQueueCreateInfo dqci = {};
    dqci.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    dqci.queueFamilyIndex = m_graphicsQueueIndex;
    dqci.queueCount = 1;
    float queuePriorities[] = { 1.0f };
    dqci.pQueuePriorities = queuePriorities;

    VkDeviceCreateInfo dci = {};
    dci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	dci.pNext = nullptr;
    dci.queueCreateInfoCount = 1;
    dci.pQueueCreateInfos = &dqci;
	dci.enabledLayerCount = (uint32_t)validationLayers.size();
	dci.ppEnabledLayerNames = validationLayers.c_ptr();
    dci.enabledExtensionCount = sizeof_array(c_deviceExtensions);
    dci.ppEnabledExtensionNames = c_deviceExtensions;

    VkPhysicalDeviceFeatures features = {};
    features.shaderClipDistance = VK_TRUE;
    dci.pEnabledFeatures = &features;

    if (vkCreateDevice(m_physicalDevice, &dci, 0, &m_logicalDevice) != VK_SUCCESS)
	{
		log::error << L"Failed to create Vulkan; unable to create logical device." << Endl;
		return false;
	}

	// Get opaque queues.
	vkGetDeviceQueue(m_logicalDevice, m_graphicsQueueIndex, 0, &m_graphicsQueue);
	vkGetDeviceQueue(m_logicalDevice, m_computeQueueIndex, 0, &m_computeQueue);

	// Create graphics command pool.
	VkCommandPoolCreateInfo cpci = {};
	cpci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cpci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	cpci.queueFamilyIndex = m_graphicsQueueIndex;

	if (vkCreateCommandPool(m_logicalDevice, &cpci, 0, &m_graphicsCommandPool) != VK_SUCCESS)
	{
		log::error << L"Failed to create Vulkan; unable to create graphics command pool." << Endl;
		return false;
	}

	// Create memory allocator.
	VmaVulkanFunctions vf = {};
	vf.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
	vf.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
	vf.vkAllocateMemory = vkAllocateMemory;
	vf.vkFreeMemory = vkFreeMemory;
	vf.vkMapMemory = vkMapMemory;
	vf.vkUnmapMemory = vkUnmapMemory;
	vf.vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges;
	vf.vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges;
	vf.vkBindBufferMemory = vkBindBufferMemory;
	vf.vkBindImageMemory = vkBindImageMemory;
	vf.vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements;
	vf.vkGetImageMemoryRequirements = vkGetImageMemoryRequirements;
	vf.vkCreateBuffer = vkCreateBuffer;
	vf.vkDestroyBuffer = vkDestroyBuffer;
	vf.vkCreateImage = vkCreateImage;
	vf.vkDestroyImage = vkDestroyImage;
	vf.vkCmdCopyBuffer = vkCmdCopyBuffer;
	// if (hasGetMemReq2 && hasDedicatedAllocation) {
	// 	vf.vkGetBufferMemoryRequirements2KHR = reinterpret_cast<PFN_vkGetBufferMemoryRequirements2KHR>(vk::GetDeviceProcAddr(m_dev, "vkGetBufferMemoryRequirements2KHR"));
	// 	vf.vkGetImageMemoryRequirements2KHR = reinterpret_cast<PFN_vkGetImageMemoryRequirements2KHR>(vk::GetDeviceProcAddr(m_dev, "vkGetImageMemoryRequirements2KHR"));
	// }

	VmaAllocatorCreateInfo aci = {};
	aci.physicalDevice = m_physicalDevice;
	aci.device = m_logicalDevice;
	aci.pVulkanFunctions = &vf;
	if (vmaCreateAllocator(&aci, &m_allocator) != VK_SUCCESS)
	{
		log::error << L"Failed to create Vulkan; failed to create allocator." << Endl;
		return false;
	}

	log::info << L"Vulkan render system created successfully." << Endl;
	return true;
}

void RenderSystemVk::destroy()
{
	finalizeVulkanApi();
}

bool RenderSystemVk::reset(const RenderSystemDesc& desc)
{
	// \todo Update mipmap bias and maximum anisotropy.
	return true;
}

void RenderSystemVk::getInformation(RenderSystemInformation& outInfo) const
{
	outInfo.dedicatedMemoryTotal = 0;
	outInfo.sharedMemoryTotal = 0;
	outInfo.dedicatedMemoryAvailable = 0;
	outInfo.sharedMemoryAvailable = 0;
}

uint32_t RenderSystemVk::getDisplayModeCount() const
{
#if defined(_WIN32)
	uint32_t count = 0;

	DEVMODE dmgl;
	std::memset(&dmgl, 0, sizeof(dmgl));
	dmgl.dmSize = sizeof(dmgl);

	while (EnumDisplaySettings(nullptr, count, &dmgl))
		++count;

	return count;
#else
	return 0;
#endif
}

DisplayMode RenderSystemVk::getDisplayMode(uint32_t index) const
{
#if defined(_WIN32)
	DEVMODE dmgl;
	std::memset(&dmgl, 0, sizeof(dmgl));
	dmgl.dmSize = sizeof(dmgl);
	EnumDisplaySettings(nullptr, index, &dmgl);

	DisplayMode dm;
	dm.width = dmgl.dmPelsWidth;
	dm.height = dmgl.dmPelsHeight;
	dm.refreshRate = (uint16_t)dmgl.dmDisplayFrequency;
	dm.colorBits = (uint16_t)dmgl.dmBitsPerPel;
	return dm;
#else
	return DisplayMode();
#endif
}

DisplayMode RenderSystemVk::getCurrentDisplayMode() const
{
#if defined(_WIN32)
	DEVMODE dmgl;
	std::memset(&dmgl, 0, sizeof(dmgl));
	dmgl.dmSize = sizeof(dmgl);
	EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &dmgl);

	DisplayMode dm;
	dm.width = dmgl.dmPelsWidth;
	dm.height = dmgl.dmPelsHeight;
	dm.refreshRate = (uint16_t)dmgl.dmDisplayFrequency;
	dm.colorBits = (uint16_t)dmgl.dmBitsPerPel;
	return dm;
#elif defined(__LINUX__)
	int screen = DefaultScreen(m_display);
	DisplayMode dm;
	dm.width = DisplayWidth(m_display, screen);
	dm.height = DisplayHeight(m_display, screen);
	dm.refreshRate = 60;
	dm.colorBits = 32;
	return dm;
#elif defined(__ANDROID__)
	DisplayMode dm;
	dm.width = m_screenWidth;
	dm.height = m_screenHeight;
	dm.refreshRate = 60;
	dm.colorBits = 32;
	return dm;
#elif defined(__IOS__)
	DisplayMode dm;
	dm.width = getScreenWidth();
	dm.height = getScreenHeight();
	dm.refreshRate = 60;
	dm.colorBits = 32;
	return dm;
#else
	return DisplayMode();
#endif
}

float RenderSystemVk::getDisplayAspectRatio() const
{
	return 0.0f;
}

Ref< IRenderView > RenderSystemVk::createRenderView(const RenderViewDefaultDesc& desc)
{
	Ref< RenderViewVk > renderView = new RenderViewVk(
		m_instance,
		m_physicalDevice,
		m_logicalDevice,
		m_allocator,
		m_graphicsQueueIndex,
		m_computeQueueIndex
	);
	if (renderView->create(desc))
		return renderView;
	else
		return nullptr;
}

Ref< IRenderView > RenderSystemVk::createRenderView(const RenderViewEmbeddedDesc& desc)
{
	Ref< RenderViewVk > renderView = new RenderViewVk(
		m_instance,
		m_physicalDevice,
		m_logicalDevice,
		m_allocator,
		m_graphicsQueueIndex,
		m_computeQueueIndex
	);
	if (renderView->create(desc))
		return renderView;
	else
		return nullptr;
}

Ref< VertexBuffer > RenderSystemVk::createVertexBuffer(const AlignedVector< VertexElement >& vertexElements, uint32_t bufferSize, bool dynamic)
{
	VkBuffer vertexBuffer = 0;

	VkBufferCreateInfo bci = {};
	bci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bci.size = bufferSize;
	bci.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	bci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VmaAllocationCreateInfo aci = {};
	aci.usage = VMA_MEMORY_USAGE_CPU_TO_GPU; // \tbd VMA_MEMORY_USAGE_GPU_ONLY;

	VmaAllocation allocation;
	if (vmaCreateBuffer(m_allocator, &bci, &aci, &vertexBuffer, &allocation, nullptr) != VK_SUCCESS)
		return nullptr;

	VkVertexInputBindingDescription vibd = {};
	vibd.binding = 0;
	vibd.stride = getVertexSize(vertexElements);
	vibd.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	AlignedVector< VkVertexInputAttributeDescription > vads;
	for (auto ve : vertexElements)
	{
		auto& vad = vads.push_back();
		vad.location = VertexAttributesVk::getLocation(ve.getDataUsage(), ve.getIndex());
		vad.binding = 0;
		vad.format = c_vkVertexElementFormats[ve.getDataType()];
		vad.offset = ve.getOffset();
	}

	// Calculate hash of vertex declaration.
	Adler32 cs;
	cs.begin();
	cs.feed(vertexElements.c_ptr(), vertexElements.size() * sizeof(VertexElement));
	cs.end();

	return new VertexBufferDynamicVk(
		bufferSize,
		m_allocator,
		allocation,
		vertexBuffer,
		vibd,
		vads,
		cs.get()
	);
}

Ref< IndexBuffer > RenderSystemVk::createIndexBuffer(IndexType indexType, uint32_t bufferSize, bool dynamic)
{
	VkBuffer indexBuffer = 0;

	VkBufferCreateInfo bci = {};
	bci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bci.size = bufferSize;
	bci.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	bci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VmaAllocationCreateInfo aci = {};
	aci.usage = VMA_MEMORY_USAGE_CPU_TO_GPU; // \tbd VMA_MEMORY_USAGE_GPU_ONLY;

	VmaAllocation allocation;
	if (vmaCreateBuffer(m_allocator, &bci, &aci, &indexBuffer, &allocation, nullptr) != VK_SUCCESS)
		return nullptr;

	return new IndexBufferVk(indexType, bufferSize, m_allocator, allocation, indexBuffer);
}

Ref< StructBuffer > RenderSystemVk::createStructBuffer(const AlignedVector< StructElement >& structElements, uint32_t bufferSize)
{
	VkBuffer storageBuffer = 0;

	VkBufferCreateInfo bci = {};
	bci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bci.size = bufferSize;
	bci.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	bci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VmaAllocationCreateInfo aci = {};
	aci.usage = VMA_MEMORY_USAGE_CPU_TO_GPU; // \tbd VMA_MEMORY_USAGE_GPU_ONLY;

	VmaAllocation allocation;
	if (vmaCreateBuffer(m_allocator, &bci, &aci, &storageBuffer, &allocation, nullptr) != VK_SUCCESS)
		return nullptr;	

	return new StructBufferVk(bufferSize, m_allocator, allocation, storageBuffer);
}

Ref< ISimpleTexture > RenderSystemVk::createSimpleTexture(const SimpleTextureCreateDesc& desc, const wchar_t* const tag)
{
	Ref< SimpleTextureVk > texture = new SimpleTextureVk(
		m_physicalDevice,
		m_logicalDevice,
		m_allocator
	);
	if (texture->create(
		m_graphicsCommandPool,
		m_graphicsQueue,
		desc,
		tag
	))
		return texture;
	else
		return nullptr;
}

Ref< ICubeTexture > RenderSystemVk::createCubeTexture(const CubeTextureCreateDesc& desc, const wchar_t* const tag)
{
	Ref< CubeTextureVk > texture = new CubeTextureVk(
		m_physicalDevice,
		m_logicalDevice,
		m_allocator,
		m_graphicsCommandPool,
		m_graphicsQueue,
		desc
	);
	if (texture->create(tag))
		return texture;
	else
		return nullptr;
}

Ref< IVolumeTexture > RenderSystemVk::createVolumeTexture(const VolumeTextureCreateDesc& desc, const wchar_t* const tag)
{
	Ref< VolumeTextureVk > texture = new VolumeTextureVk();
	if (texture->create(
		m_physicalDevice,
		m_logicalDevice,
		m_graphicsCommandPool,
		m_graphicsQueue,
		desc,
		tag
	))
		return texture;
	else
		return nullptr;
}

Ref< RenderTargetSet > RenderSystemVk::createRenderTargetSet(const RenderTargetSetCreateDesc& desc, const wchar_t* const tag)
{
	Ref< RenderTargetSetVk > renderTargetSet = new RenderTargetSetVk(
		m_physicalDevice,
		m_logicalDevice,
		m_allocator,
		m_graphicsCommandPool,
		m_graphicsQueue
	);
	if (renderTargetSet->create(desc, tag))
		return renderTargetSet;
	else
		return nullptr;
}

Ref< IProgram > RenderSystemVk::createProgram(const ProgramResource* programResource, const wchar_t* const tag)
{
	Ref< const ProgramResourceVk > resource = dynamic_type_cast< const ProgramResourceVk* >(programResource);
	if (!resource)
		return nullptr;

	Ref< ProgramVk > program = new ProgramVk(
		m_physicalDevice,
		m_logicalDevice,
		m_allocator
	);
	if (program->create(resource, tag))
		return program;
	else
		return nullptr;
}

Ref< ITimeQuery > RenderSystemVk::createTimeQuery() const
{
	Ref< TimeQueryVk > timeQuery = new TimeQueryVk();
	if (timeQuery->create())
		return timeQuery;
	else
		return nullptr;
}

void RenderSystemVk::purge()
{
}

void RenderSystemVk::getStatistics(RenderSystemStatistics& outStatistics) const
{
}

	}
}
