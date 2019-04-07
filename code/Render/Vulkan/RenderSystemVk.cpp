#include <cstring>
#include "Core/Log/Log.h"
#include "Core/Misc/Adler32.h"
#include "Core/Misc/Align.h"
#include "Core/Misc/AutoPtr.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/TString.h"
#include "Render/VertexElement.h"
#include "Render/Vulkan/ApiLoader.h"
#include "Render/Vulkan/ContextVk.h"
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
#include "Render/Vulkan/TimeQueryVk.h"
#include "Render/Vulkan/UtilitiesVk.h"
#include "Render/Vulkan/VertexAttributesVk.h"
#include "Render/Vulkan/VertexBufferVk.h"
#include "Render/Vulkan/VolumeTextureVk.h"
#include "Render/Vulkan/Editor/Glsl/GlslType.h"
#if defined(_WIN32)
#	include "Render/Vulkan/Win32/Window.h"
#elif defined(__LINUX__)
#	include "Render/Vulkan/Linux/Window.h"
#endif

namespace traktor
{
	namespace render
	{
		namespace
		{

const char* c_validationLayerNames[] = /*{ "VK_LAYER_RENDERDOC_Capture" };*/ { "VK_LAYER_LUNARG_standard_validation" };
#if defined(_WIN32)
const char* c_extensions[] = { "VK_KHR_surface", "VK_KHR_win32_surface", "VK_EXT_debug_report" };
#elif defined(__LINUX__)
const char* c_extensions[] = { "VK_KHR_surface", "VK_KHR_xlib_surface", "VK_EXT_debug_report" };
#else
const char* c_extensions[] = { "VK_KHR_surface", "VK_EXT_debug_report" };
#endif
const char* c_deviceExtensions[] = { "VK_KHR_swapchain" };

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.RenderSystemVk", 0, RenderSystemVk, IRenderSystem)

RenderSystemVk::RenderSystemVk()
#if defined(__LINUX__)
:	m_display(nullptr)
,	m_instance(nullptr)
#else
:	m_instance(nullptr)
#endif
,	m_physicalDevice(nullptr)
,	m_logicalDevice(nullptr)
,	m_graphicsQueueIndex(~0)
,	m_graphicsQueue(nullptr)
,	m_commandPool(nullptr)
,	m_setupCommandBuffer(nullptr)
,	m_haveValidationLayer(false)
{
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
#endif

#if defined(_WIN32) || defined(__LINUX__)
	if (!initializeVulkanApi())
		return false;
#endif

	// Check for validation layers.
	uint32_t layerCount = 0;
	vkEnumerateInstanceLayerProperties(&layerCount, 0);

	AutoArrayPtr< VkLayerProperties > layersAvailable(new VkLayerProperties[layerCount]);
	vkEnumerateInstanceLayerProperties(&layerCount, layersAvailable.ptr());

	log::info << IncreaseIndent;
	for (uint32_t i = 0; i < layerCount; ++i)
	{
		if (std::strcmp(layersAvailable[i].layerName, c_validationLayerNames[0]) == 0)
			m_haveValidationLayer = true;
	}
	log::info << DecreaseIndent;
	if (!m_haveValidationLayer)
		log::warning << L"No Vulkan validation layer found." << Endl;

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
	ii.enabledLayerCount = 0;
	ii.ppEnabledLayerNames = nullptr;
	ii.enabledExtensionCount = sizeof_array(c_extensions);
	ii.ppEnabledExtensionNames = c_extensions;
	if (m_haveValidationLayer)
	{
		ii.enabledLayerCount = sizeof_array(c_validationLayerNames);
		ii.ppEnabledLayerNames = c_validationLayerNames;
	}

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

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, 0);

	AutoArrayPtr< VkQueueFamilyProperties > queueFamilyProperties(new VkQueueFamilyProperties[queueFamilyCount]);
	vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, queueFamilyProperties.ptr());

	for (uint32_t i = 0; i < queueFamilyCount; ++i)
	{
		if (queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			m_graphicsQueueIndex = i;
			break;
		}
	}
	if (m_graphicsQueueIndex == ~0)
	{
		log::error << L"Failed to create Vulkan; no suitable graphics queue found." << Endl;
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
    dci.enabledLayerCount = 0;
    dci.ppEnabledLayerNames = nullptr;
    dci.enabledExtensionCount = sizeof_array(c_deviceExtensions);
    dci.ppEnabledExtensionNames = c_deviceExtensions;
	if (m_haveValidationLayer)
	{
		dci.enabledLayerCount = sizeof_array(c_validationLayerNames);
		dci.ppEnabledLayerNames = c_validationLayerNames;
	}

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

	// Create command pool.
	VkCommandPoolCreateInfo cpci = {};
	cpci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cpci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	cpci.queueFamilyIndex = m_graphicsQueueIndex;

	if (vkCreateCommandPool(m_logicalDevice, &cpci, 0, &m_commandPool) != VK_SUCCESS)
	{
		log::error << L"Failed to create Vulkan; unable to create command pool." << Endl;
		return false;
	}

	// Create command buffers from pool.
	VkCommandBufferAllocateInfo cbai = {};
	cbai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cbai.commandPool = m_commandPool;
	cbai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cbai.commandBufferCount = 1;

	if (vkAllocateCommandBuffers(m_logicalDevice, &cbai, &m_setupCommandBuffer) != VK_SUCCESS)
	{
		log::error << L"Failed to create Vulkan; failed to allocate setup command buffer." << Endl;
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
#else
	return DisplayMode();
#endif
}

float RenderSystemVk::getDisplayAspectRatio() const
{
	return 1.0f;
}

Ref< IRenderView > RenderSystemVk::createRenderView(const RenderViewDefaultDesc& desc)
{
	Ref< RenderViewVk > renderView = new RenderViewVk(
		m_instance,
		m_physicalDevice,
		m_logicalDevice,
		m_graphicsQueueIndex
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
		m_graphicsQueueIndex
	);
	if (renderView->create(desc))
		return renderView;
	else
		return nullptr;
}

Ref< VertexBuffer > RenderSystemVk::createVertexBuffer(const AlignedVector< VertexElement >& vertexElements, uint32_t bufferSize, bool dynamic)
{
	VkBuffer vertexBuffer = nullptr;

	VkBufferCreateInfo bci = {};
	bci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bci.size = bufferSize;
	bci.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	bci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(m_logicalDevice, &bci, nullptr, &vertexBuffer) != VK_SUCCESS)
		return nullptr;

	VkPhysicalDeviceMemoryProperties memoryProperties = {};
	vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memoryProperties);

	VkMemoryRequirements vertexBufferMemoryRequirements = {};
	vkGetBufferMemoryRequirements(m_logicalDevice, vertexBuffer, &vertexBufferMemoryRequirements);

	VkMemoryAllocateInfo mai = {};
	mai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	mai.allocationSize = vertexBufferMemoryRequirements.size;
	mai.memoryTypeIndex = getMemoryTypeIndex(m_physicalDevice, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, vertexBufferMemoryRequirements);

	VkDeviceMemory vertexBufferMemory;
	if (vkAllocateMemory(m_logicalDevice, &mai, nullptr, &vertexBufferMemory) != VK_SUCCESS)
		return nullptr;

	VkVertexInputBindingDescription vibd = {};
	vibd.binding = 0;
	vibd.stride = getVertexSize(vertexElements);
	vibd.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	AlignedVector< VkVertexInputAttributeDescription > vertexAttributeDescriptions;
	for (auto ve : vertexElements)
	{
		const VkFormat c_formats[] =
		{
			VK_FORMAT_R32_SFLOAT, // DtFloat1
			VK_FORMAT_R32G32_SFLOAT, // DtFloat2
			VK_FORMAT_R32G32B32_SFLOAT, // DtFloat3
			VK_FORMAT_R32G32B32A32_SFLOAT, // DtFloat4
			VK_FORMAT_R8G8B8A8_SNORM, // DtByte4
			VK_FORMAT_R8G8B8A8_UNORM, // DtByte4N
			VK_FORMAT_R16G16_SNORM, // DtShort2
			VK_FORMAT_R16G16B16A16_SNORM, // DtShort4
			VK_FORMAT_R16G16_UNORM, // DtShort2N
			VK_FORMAT_R16G16B16A16_UNORM, // DtShort4N
			VK_FORMAT_R16G16_SFLOAT, // DtHalf2
			VK_FORMAT_R16G16B16A16_SFLOAT // DtHalf4
		};

		VkVertexInputAttributeDescription vertexAttributeDescription = {};
		vertexAttributeDescription.location = VertexAttributesVk::getLocation(ve.getDataUsage(), ve.getIndex());
		vertexAttributeDescription.binding = 0;
		vertexAttributeDescription.format = c_formats[ve.getDataType()];
		vertexAttributeDescription.offset = ve.getOffset();
		vertexAttributeDescriptions.push_back(vertexAttributeDescription);
	}

	// Calculate hash of vertex declaration.
	Adler32 cs;
	cs.begin();
	cs.feed(vertexElements.c_ptr(), vertexElements.size() * sizeof(VertexElement));
	cs.end();

	return new VertexBufferVk(
		bufferSize,
		m_logicalDevice,
		vertexBuffer,
		vertexBufferMemory,
		vibd,
		vertexAttributeDescriptions,
		cs.get()
	);
}

Ref< IndexBuffer > RenderSystemVk::createIndexBuffer(IndexType indexType, uint32_t bufferSize, bool dynamic)
{
	VkBuffer indexBuffer = nullptr;

	VkBufferCreateInfo bci = {};
	bci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bci.size = bufferSize;
	bci.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	bci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(m_logicalDevice, &bci, nullptr, &indexBuffer) != VK_SUCCESS)
		return nullptr;

	VkPhysicalDeviceMemoryProperties memoryProperties = {};
	vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memoryProperties);

	VkMemoryRequirements indexBufferMemoryRequirements = {};
	vkGetBufferMemoryRequirements(m_logicalDevice, indexBuffer, &indexBufferMemoryRequirements);

	VkMemoryAllocateInfo mai = {};
	mai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	mai.allocationSize = indexBufferMemoryRequirements.size;
 	mai.memoryTypeIndex = getMemoryTypeIndex(m_physicalDevice, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, indexBufferMemoryRequirements);

	VkDeviceMemory indexBufferMemory;
	if (vkAllocateMemory(m_logicalDevice, &mai, nullptr, &indexBufferMemory) != VK_SUCCESS)
		return nullptr;

	return new IndexBufferVk(indexType, bufferSize, m_logicalDevice, indexBuffer, indexBufferMemory);
}

Ref< StructBuffer > RenderSystemVk::createStructBuffer(const AlignedVector< StructElement >& structElements, uint32_t bufferSize)
{
	return nullptr;
}

Ref< ISimpleTexture > RenderSystemVk::createSimpleTexture(const SimpleTextureCreateDesc& desc)
{
	Ref< SimpleTextureVk > texture = new SimpleTextureVk();
	if (texture->create(
		m_physicalDevice,
		m_logicalDevice,
		m_commandPool,
		m_graphicsQueue,
		desc
	))
		return texture;
	else
		return nullptr;
}

Ref< ICubeTexture > RenderSystemVk::createCubeTexture(const CubeTextureCreateDesc& desc)
{
	Ref< CubeTextureVk > texture = new CubeTextureVk();
	if (texture->create(
		m_physicalDevice,
		m_logicalDevice,
		m_commandPool,
		m_graphicsQueue,
		desc
	))
		return texture;
	else
		return nullptr;
}

Ref< IVolumeTexture > RenderSystemVk::createVolumeTexture(const VolumeTextureCreateDesc& desc)
{
	Ref< VolumeTextureVk > texture = new VolumeTextureVk();
	if (texture->create(
		m_physicalDevice,
		m_logicalDevice,
		m_commandPool,
		m_graphicsQueue,
		desc
	))
		return texture;
	else
		return nullptr;
}

Ref< RenderTargetSet > RenderSystemVk::createRenderTargetSet(const RenderTargetSetCreateDesc& desc)
{
	Ref< RenderTargetSetVk > renderTargetSet = new RenderTargetSetVk();
	if (renderTargetSet->create(m_physicalDevice, m_logicalDevice, desc))
		return renderTargetSet;
	else
		return nullptr;
}

Ref< IProgram > RenderSystemVk::createProgram(const ProgramResource* programResource, const wchar_t* const tag)
{
	Ref< const ProgramResourceVk > resource = dynamic_type_cast< const ProgramResourceVk* >(programResource);
	if (!resource)
		return nullptr;

	Ref< ProgramVk > program = new ProgramVk();
	if (program->create(m_physicalDevice, m_logicalDevice, resource))
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
