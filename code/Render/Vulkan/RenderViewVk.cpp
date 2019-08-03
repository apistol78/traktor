#include "Core/Log/Log.h"
#include "Core/Misc/AutoPtr.h"
#include "Core/Misc/SafeDestroy.h"
#include "Render/Vulkan/ApiLoader.h"
#include "Render/Vulkan/IndexBufferVk.h"
#include "Render/Vulkan/ProgramVk.h"
#include "Render/Vulkan/RenderTargetDepthVk.h"
#include "Render/Vulkan/RenderTargetVk.h"
#include "Render/Vulkan/RenderTargetSetVk.h"
#include "Render/Vulkan/RenderViewVk.h"
#include "Render/Vulkan/UniformBufferPoolVk.h"
#include "Render/Vulkan/UtilitiesVk.h"
#include "Render/Vulkan/VertexBufferVk.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

struct RenderEventTypePred
{
	RenderEventType m_type;

	RenderEventTypePred(RenderEventType type)
	:	m_type(type)
	{
	}

	bool operator () (const RenderEvent& evt) const
	{
		return evt.type == m_type;
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderViewVk", RenderViewVk, IRenderView)

RenderViewVk::RenderViewVk(
	VkInstance instance,
	VkPhysicalDevice physicalDevice,
	VkDevice logicalDevice,
	uint32_t graphicsQueueIndex,
	uint32_t computeQueueIndex
)
:	m_instance(instance)
,	m_physicalDevice(physicalDevice)
,	m_logicalDevice(logicalDevice)
,	m_graphicsQueueIndex(graphicsQueueIndex)
,	m_computeQueueIndex(computeQueueIndex)
,	m_surface(0)
,	m_presentQueueIndex(~0)
,	m_presentQueue(0)
,	m_graphicsCommandPool(0)
,	m_computeCommandPool(0)
,	m_graphicsCommandBuffer(0)
,	m_computeCommandBuffer(0)
,	m_swapChain(0)
,	m_descriptorPool(0)
,	m_renderFence(0)
,	m_presentCompleteSemaphore(0)
,	m_haveDebugMarkers(false)
,	m_targetStateDirty(false)
,	m_targetId(0)
,	m_targetRenderPass(0)
,	m_drawCalls(0)
,	m_primitiveCount(0)
{
}

RenderViewVk::~RenderViewVk()
{
	close();
}

bool RenderViewVk::create(const RenderViewDefaultDesc& desc)
{
#if defined(_WIN32) || defined(__LINUX__)
	// Create render window.
	m_window = new Window();
	if (!m_window->create(desc.displayMode.width, desc.displayMode.height))
	{
		log::error << L"Failed to create render view; unable to create window." << Endl;
		return false;
	}
	m_window->setTitle(!desc.title.empty() ? desc.title.c_str() : L"Traktor - Vulkan Renderer");
	m_window->show();
#endif
#if defined(_WIN32)
	if (m_window)
		m_window->addListener(this);
#endif

	// Create renderable surface.
#if defined(_WIN32)
    VkWin32SurfaceCreateInfoKHR sci = {};
    sci.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    sci.hinstance = GetModuleHandle(nullptr);
    sci.hwnd = (HWND)*m_window;
    if (vkCreateWin32SurfaceKHR(m_instance, &sci, nullptr, &m_surface) != VK_SUCCESS)
	{
		log::error << L"Failed to create Vulkan; unable to create Win32 renderable surface." << Endl;
		return false;
	}
#elif defined(__LINUX__)
	VkXlibSurfaceCreateInfoKHR sci = {};
	sci.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
	sci.dpy = m_window->getDisplay();
	sci.window = m_window->getWindow();
    if (vkCreateXlibSurfaceKHR(m_instance, &sci, nullptr, &m_surface) != VK_SUCCESS)
	{
		log::error << L"Failed to create Vulkan; unable to create X11 renderable surface." << Endl;
		return false;
	}
#endif

	if (!create(desc.displayMode.width, desc.displayMode.height))
		return false;

	return true;	
}

bool RenderViewVk::create(const RenderViewEmbeddedDesc& desc)
{
	int32_t width = 64;
	int32_t height = 64;

	// Create renderable surface.
#if defined(_WIN32)
    VkWin32SurfaceCreateInfoKHR sci = {};
    sci.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    sci.hinstance = GetModuleHandle(nullptr);
    sci.hwnd = desc.syswin.hWnd;
    if (vkCreateWin32SurfaceKHR(m_instance, &sci, nullptr, &m_surface) != VK_SUCCESS)
	{
		log::error << L"Failed to create Vulkan; unable to create Win32 renderable surface." << Endl;
		return false;
	}
#elif defined(__LINUX__)
	VkXlibSurfaceCreateInfoKHR sci = {};
	sci.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
	sci.dpy = (::Display*)desc.syswin.display;
	sci.window = desc.syswin.window;
    if (vkCreateXlibSurfaceKHR(m_instance, &sci, nullptr, &m_surface) != VK_SUCCESS)
	{
		log::error << L"Failed to create Vulkan; unable to create X11 renderable surface." << Endl;
		return false;
	}
#elif defined(__ANDROID__)
	VkAndroidSurfaceCreateInfoKHR sci = {};
	sci.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
	sci.flags = 0;
	sci.window = *desc.syswin.window;
	if (vkCreateAndroidSurfaceKHR(m_instance, &sci, nullptr, &m_surface) != VK_SUCCESS)
	{
		log::error << L"Failed to create Vulkan; unable to create Android renderable surface." << Endl;
		return false;
	}

	width = ANativeWindow_getWidth(sci.window);
	height = ANativeWindow_getHeight(sci.window);
#endif

	if (!create(width, height))
		return false;

	return true;
}

bool RenderViewVk::nextEvent(RenderEvent& outEvent)
{
#if defined(_WIN32)
	MSG msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	if (!m_eventQueue.empty())
	{
		outEvent = m_eventQueue.front();
		m_eventQueue.pop_front();
		return true;
	}
	else
		return false;
#elif defined(__LINUX__)
	return m_window ? m_window->update(outEvent) : false;
#else
	return false;
#endif
}

void RenderViewVk::close()
{
 //   vkDestroySemaphore(m_logicalDevice, m_presentCompleteSemaphore, nullptr);
	//vkDestroyFence(m_logicalDevice, m_renderFence, nullptr);	
}

bool RenderViewVk::reset(const RenderViewDefaultDesc& desc)
{
#if defined(_WIN32) || defined(__LINUX__)
	// Cannot reset embedded view.
	if (!m_window)
		return false;

#	if defined(_WIN32)
	m_window->removeListener(this);
#	endif

	m_window->setTitle(!desc.title.empty() ? desc.title.c_str() : L"Traktor - Vulkan Renderer");

#	if defined(_WIN32)
	m_window->addListener(this);
#	endif
#endif

	if (!reset(
		desc.displayMode.width,
		desc.displayMode.height
	))
		return false;

	return true;
}

bool RenderViewVk::reset(int32_t width, int32_t height)
{
	vkDeviceWaitIdle(m_logicalDevice);

	// Destroy primary targets.
	for (auto primaryTarget : m_primaryTargets)
		primaryTarget->destroy();
	m_primaryTargets.resize(0);

	// Destroy previous swap chain.
	if (m_swapChain != 0)
	{
		vkDestroySwapchainKHR(m_logicalDevice, m_swapChain, 0);	
		m_swapChain = 0;
	}

	if (create(width, height))
		return true;
	else
		return false;
}

int RenderViewVk::getWidth() const
{
	return m_primaryTargets.front()->getWidth();
}

int RenderViewVk::getHeight() const
{
	return m_primaryTargets.front()->getHeight();
}

bool RenderViewVk::isActive() const
{
#if defined(_WIN32) || defined(__ANDROID__)
	return true;
#else
	return m_window->isActive();
#endif
}

bool RenderViewVk::isMinimized() const
{
	return false;
}

bool RenderViewVk::isFullScreen() const
{
#if defined(_WIN32)
	return m_window->haveFullScreenStyle();
#elif defined(__ANDROID__)
	return true;
#else
	return m_window->isFullScreen();
#endif
}

void RenderViewVk::showCursor()
{
}

void RenderViewVk::hideCursor()
{
}

bool RenderViewVk::isCursorVisible() const
{
	return false;
}

bool RenderViewVk::setGamma(float gamma)
{
	return false;
}

void RenderViewVk::setViewport(const Viewport& viewport)
{
	T_ASSERT(viewport.width > 0);
	T_ASSERT(viewport.height > 0);

	if (m_targetStateStack.empty())
	{
		m_viewport = viewport;
	}
	else
	{
		m_targetStateStack.back().viewport = viewport;

		if (!m_targetStateDirty)
		{
			VkViewport vp = {};
			vp.x = viewport.left;
			vp.y = viewport.top;
			vp.width = viewport.width;
			vp.height = viewport.height;
			vp.minDepth = viewport.nearZ;
			vp.maxDepth = viewport.farZ;
			vkCmdSetViewport(m_graphicsCommandBuffer, 0, 1, &vp);
		}
	}
}

Viewport RenderViewVk::getViewport()
{
	return m_targetStateStack.empty() ? m_viewport : m_targetStateStack.back().viewport;
}

SystemWindow RenderViewVk::getSystemWindow()
{
#if defined(_WIN32)
	return SystemWindow(*m_window);
#elif defined(__LINUX__)
	return SystemWindow(m_window->getDisplay(), m_window->getWindow());
#else
	return SystemWindow();
#endif
}

bool RenderViewVk::begin(
	const Clear* clear
)
{
	// Get next target from swap chain.
    vkAcquireNextImageKHR(
		m_logicalDevice,
		m_swapChain,
		UINT64_MAX,
		m_presentCompleteSemaphore,
		VK_NULL_HANDLE,
		&m_currentImageIndex
	);

	// Reset descriptor pool.
	if (vkResetDescriptorPool(m_logicalDevice, m_descriptorPool, 0) != VK_SUCCESS)
		return false;

	// Begin recording command buffer.
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	if (vkBeginCommandBuffer(m_graphicsCommandBuffer, &beginInfo) != VK_SUCCESS)
		return false;

	// Push primary target onto stack.
	TargetState ts;
	ts.rts = m_primaryTargets[m_currentImageIndex];
	ts.colorIndex = 0;
	ts.clear.mask = 0;
	ts.viewport = m_viewport;

	if (clear)
		ts.clear = *clear;

	m_targetStateStack.push_back(ts);
	m_targetStateDirty = true;

	m_drawCalls = 0;
	m_primitiveCount = 0;
	return true;
}

bool RenderViewVk::begin(
	RenderTargetSet* renderTargetSet,
	const Clear* clear
)
{
	// End current render pass, restart later.
	// \tbd
	// This indicate poor use of render passes,
	// some draw calls on pass before and after
	// nested begin/end.
	// Also this will cause viewport to be
	// reset along with other stuff.
	// Also targets transition to "target" multiple
	// times.
	if (!m_targetStateDirty)
		vkCmdEndRenderPass(m_graphicsCommandBuffer);

	TargetState ts;
	ts.rts = mandatory_non_null_type_cast< RenderTargetSetVk* >(renderTargetSet);
	ts.colorIndex = -1;
	ts.clear.mask = 0;
	ts.viewport = Viewport(0, 0, ts.rts->getWidth(), ts.rts->getHeight(), 0.0f, 1.0f);

	if (clear)
		ts.clear = *clear;

	m_targetStateStack.push_back(ts);
	m_targetStateDirty = true;
	return true;
}

bool RenderViewVk::begin(
	RenderTargetSet* renderTargetSet,
	int32_t renderTarget,
	const Clear* clear
)
{
	// End current render pass, restart later.
	if (!m_targetStateDirty)
		vkCmdEndRenderPass(m_graphicsCommandBuffer);

	TargetState ts;
	ts.rts = mandatory_non_null_type_cast< RenderTargetSetVk* >(renderTargetSet);
	ts.colorIndex = renderTarget;
	ts.clear.mask = 0;
	ts.viewport = Viewport(0, 0, ts.rts->getWidth(), ts.rts->getHeight(), 0.0f, 1.0f);

	if (clear)
		ts.clear = *clear;

	m_targetStateStack.push_back(ts);
	m_targetStateDirty = true;
	return true;
}

void RenderViewVk::draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives)
{
	draw(vertexBuffer, indexBuffer, program, primitives, 1);
}

void RenderViewVk::draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives, uint32_t instanceCount)
{
	TargetState& ts = m_targetStateStack.back();

	VertexBufferVk* vb = mandatory_non_null_type_cast< VertexBufferVk* >(vertexBuffer);
	ProgramVk* p = mandatory_non_null_type_cast< ProgramVk* >(program);

	validateTargetState();
	validatePipeline(vb, p, primitives.type);

	float targetSize[] =
	{
		(float)ts.rts->getWidth(),
		(float)ts.rts->getHeight()
	};
	p->validateGraphics(m_logicalDevice, m_descriptorPool, m_graphicsCommandBuffer, m_uniformBufferPool, targetSize);

	const uint32_t c_primitiveMul[] = { 1, 0, 2, 2, 3 };
	uint32_t vertexCount = primitives.count * c_primitiveMul[primitives.type];

	VkBuffer vbb = vb->getVkBuffer();
	VkDeviceSize offsets = {};
	vkCmdBindVertexBuffers(m_graphicsCommandBuffer, 0, 1, &vbb, &offsets);

	if (indexBuffer && primitives.indexed)
	{
		IndexBufferVk* ib = mandatory_non_null_type_cast< IndexBufferVk* >(indexBuffer);
		VkBuffer ibb = ib->getVkBuffer();

		VkDeviceSize offset = {};
		vkCmdBindIndexBuffer(
			m_graphicsCommandBuffer,
			ibb,
			offset,
			(ib->getIndexType() == ItUInt16) ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32
		);

		vkCmdDrawIndexed(
			m_graphicsCommandBuffer,
			vertexCount,	// index count
			instanceCount,	// instance count
			primitives.offset,	// first index
			0,	// vertex offset
			0	// first instance id
		);
	}
	else
	{
		vkCmdDraw(
			m_graphicsCommandBuffer,
			vertexCount,   // vertex count
			instanceCount,   // instance count
			primitives.offset,   // first vertex
			0 // first instance
		);
	}

	m_drawCalls++;
	m_primitiveCount += primitives.count * instanceCount;
}

void RenderViewVk::compute(IProgram* program, const int32_t* workSize)
{
	ProgramVk* p = mandatory_non_null_type_cast< ProgramVk* >(program);
	p->validateCompute(m_logicalDevice, m_descriptorPool, m_computeCommandBuffer, m_uniformBufferPool);
	vkCmdDispatch(m_computeCommandBuffer, workSize[0], workSize[1], workSize[2]);
}

void RenderViewVk::end()
{
	validateTargetState();

	// Close current render pass.
	vkCmdEndRenderPass(m_graphicsCommandBuffer);

	// Transition target to texture if necessary.
	if (m_targetStateStack.size() >= 2)
	{
		TargetState& ts = m_targetStateStack.back();
		ts.rts->prepareAsTexture(
			m_graphicsCommandBuffer,
			ts.colorIndex
		);
	}

	// Pop previous render pass from stack.
	m_targetStateStack.pop_back();
	m_targetStateDirty = true;
}

void RenderViewVk::present()
{
	T_FATAL_ASSERT (m_targetStateStack.empty());

	// Prepare primary color for presentation.
	m_primaryTargets[m_currentImageIndex]->getColorTargetVk(0)->prepareForPresentation(m_graphicsCommandBuffer);

	// End recording command buffer.
	vkEndCommandBuffer(m_graphicsCommandBuffer);

	// Wait until GPU has finished rendering all commands.
    VkPipelineStageFlags waitStageMash = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

    VkSubmitInfo si = {};
    si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    si.waitSemaphoreCount = 1;
    si.pWaitSemaphores = &m_presentCompleteSemaphore;
    si.pWaitDstStageMask = &waitStageMash;
    si.commandBufferCount = 1;
    si.pCommandBuffers = &m_graphicsCommandBuffer;
    si.signalSemaphoreCount = 0;
    si.pSignalSemaphores = nullptr;

	vkResetFences(m_logicalDevice, 1, &m_renderFence);
    vkQueueSubmit(m_presentQueue, 1, &si, m_renderFence);
    vkWaitForFences(m_logicalDevice, 1, &m_renderFence, VK_TRUE, UINT64_MAX);

	// Queue presentation of current primary target.
    VkPresentInfoKHR pi = {};
    pi.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    pi.swapchainCount = 1;
    pi.pSwapchains = &m_swapChain;
    pi.pImageIndices = &m_currentImageIndex;
    pi.waitSemaphoreCount = 0;
    pi.pWaitSemaphores = nullptr;
    pi.pResults = nullptr;

    vkQueuePresentKHR(m_presentQueue, &pi);

	// Collect, or cycle, released buffers.
	m_uniformBufferPool->collect();
}

void RenderViewVk::pushMarker(const char* const marker)
{
#if !defined(__ANDROID__)
	if (m_haveDebugMarkers)
	{
		VkDebugMarkerMarkerInfoEXT mi = {};
		mi.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;
		mi.pMarkerName = marker;
		vkCmdDebugMarkerBeginEXT(m_graphicsCommandBuffer, &mi);
	}
#endif
}

void RenderViewVk::popMarker()
{
#if !defined(__ANDROID__)
	if (m_haveDebugMarkers)
	{
		vkCmdDebugMarkerEndEXT(m_graphicsCommandBuffer);
	}
#endif
}

void RenderViewVk::getStatistics(RenderViewStatistics& outStatistics) const
{
	outStatistics.drawCalls = m_drawCalls;
	outStatistics.primitiveCount = m_primitiveCount;
}

bool RenderViewVk::getBackBufferContent(void* buffer) const
{
	return false;
}

bool RenderViewVk::create(uint32_t width, uint32_t height)
{
	// Find present queue.
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, 0);

	AutoArrayPtr< VkQueueFamilyProperties > queueFamilyProperties(new VkQueueFamilyProperties[queueFamilyCount]);
	vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, queueFamilyProperties.ptr());

	m_presentQueueIndex = ~0;
	for (uint32_t i = 0; i < queueFamilyCount; ++i)
	{
		VkBool32 supportsPresent;
		vkGetPhysicalDeviceSurfaceSupportKHR(m_physicalDevice, i, m_surface, &supportsPresent);
		if (supportsPresent)
		{
			m_presentQueueIndex = i;
			break;
		}
	}
	if (m_presentQueueIndex == ~0)
	{
		log::error << L"Failed to create Vulkan; no suitable present queue found." << Endl;
		return false;
	}

	// Get opaque queues.
	vkGetDeviceQueue(m_logicalDevice, m_presentQueueIndex, 0, &m_presentQueue);

	// Create graphics command pool.
	VkCommandPoolCreateInfo cpci = {};
	cpci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cpci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	cpci.queueFamilyIndex = m_graphicsQueueIndex;

	if (vkCreateCommandPool(m_logicalDevice, &cpci, 0, &m_graphicsCommandPool) != VK_SUCCESS)
	{
		log::error << L"Failed to create Vulkan; unable to create command pool." << Endl;
		return false;
	}

	// Create compute command pool.
	cpci = {};
	cpci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cpci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	cpci.queueFamilyIndex = m_computeQueueIndex;

	if (vkCreateCommandPool(m_logicalDevice, &cpci, 0, &m_computeCommandPool) != VK_SUCCESS)
	{
		log::error << L"Failed to create Vulkan; unable to create compute command pool." << Endl;
		return false;
	}

	// Create graphics command buffers from pool.
	VkCommandBufferAllocateInfo cbai = {};
	cbai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cbai.commandPool = m_graphicsCommandPool;
	cbai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cbai.commandBufferCount = 1;

	if (vkAllocateCommandBuffers(m_logicalDevice, &cbai, &m_graphicsCommandBuffer) != VK_SUCCESS)
	{
		log::error << L"Failed to create Vulkan; failed to allocate graphics command buffer." << Endl;
		return false;
	}

	// Create compute command buffers from pool.
	cbai = {};
	cbai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cbai.commandPool = m_computeCommandPool;
	cbai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cbai.commandBufferCount = 1;

	if (vkAllocateCommandBuffers(m_logicalDevice, &cbai, &m_computeCommandBuffer) != VK_SUCCESS)
	{
		log::error << L"Failed to create Vulkan; failed to allocate compute command buffer." << Endl;
		return false;
	}

	// Determine primary target color format/space.
	uint32_t surfaceFormatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &surfaceFormatCount, nullptr);
	if (surfaceFormatCount == 0)
	{
		log::error << L"Failed to create Vulkan; no surface formats." << Endl;
		return false;
	}

	AutoArrayPtr< VkSurfaceFormatKHR > surfaceFormats(new VkSurfaceFormatKHR[surfaceFormatCount]);
	vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &surfaceFormatCount, surfaceFormats.ptr());

	VkFormat colorFormat = surfaceFormats[0].format;
	if (colorFormat == VK_FORMAT_UNDEFINED)
		colorFormat = VK_FORMAT_B8G8R8_UNORM;

	VkColorSpaceKHR colorSpace = surfaceFormats[0].colorSpace;

	// Determine number of images in swapchain.
	VkSurfaceCapabilitiesKHR surfaceCapabilities = {};
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice, m_surface, &surfaceCapabilities);

	uint32_t desiredImageCount = 2;
	if (desiredImageCount < surfaceCapabilities.minImageCount)
		desiredImageCount = surfaceCapabilities.minImageCount;
	else if (surfaceCapabilities.maxImageCount != 0 && desiredImageCount > surfaceCapabilities.maxImageCount)
		desiredImageCount = surfaceCapabilities.maxImageCount;

	VkExtent2D surfaceResolution =  surfaceCapabilities.currentExtent;
	if (surfaceResolution.width <= -1)
	{
		surfaceResolution.width = width;
		surfaceResolution.height = height;
	}

	VkSurfaceTransformFlagBitsKHR preTransform = surfaceCapabilities.currentTransform;
	if (surfaceCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
		preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;

	// Determine presentation mode.
	uint32_t presentModeCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, m_surface, &presentModeCount, 0);

	AutoArrayPtr< VkPresentModeKHR > presentModes(new VkPresentModeKHR[presentModeCount]);
	vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, m_surface, &presentModeCount, presentModes.ptr());

	VkPresentModeKHR presentationMode = VK_PRESENT_MODE_FIFO_KHR;   // always supported.
	for (uint32_t i = 0; i < presentModeCount; ++i)
	{
		if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			presentationMode = VK_PRESENT_MODE_MAILBOX_KHR;
			break;
		}
	}

	// Create swap chain.
	VkSwapchainCreateInfoKHR scci = {};
	scci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	scci.surface = m_surface;
	scci.minImageCount = desiredImageCount;
	scci.imageFormat = colorFormat;
	scci.imageColorSpace = colorSpace;
	scci.imageExtent = surfaceResolution;
	scci.imageArrayLayers = 1;
	scci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	scci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	scci.preTransform = preTransform;
	scci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	scci.presentMode = presentationMode;
	scci.clipped = true;

	uint32_t queueFamilyIndices[] = { m_graphicsQueueIndex, m_presentQueueIndex };
	if (m_graphicsQueueIndex != m_presentQueueIndex)
	{
		// Need to be sharing between queues in order to be presentable.
		scci.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		scci.queueFamilyIndexCount = 2;
		scci.pQueueFamilyIndices = queueFamilyIndices;
	}

	if (vkCreateSwapchainKHR(m_logicalDevice, &scci, 0, &m_swapChain) != VK_SUCCESS)
	{
		log::error << L"Failed to create Vulkan; unable to create swap chain." << Endl;
		return false;
	}

	// Get primary color images.
	uint32_t imageCount = 0;
	vkGetSwapchainImagesKHR(m_logicalDevice, m_swapChain, &imageCount, 0);

	AlignedVector< VkImage > presentImages(imageCount);
	vkGetSwapchainImagesKHR(m_logicalDevice, m_swapChain, &imageCount, presentImages.ptr());

	// Create primary depth image.
	VkImage depthImage = 0;

	VkImageCreateInfo ici = {};
	ici.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	ici.imageType = VK_IMAGE_TYPE_2D;
	ici.format = VK_FORMAT_D24_UNORM_S8_UINT;
	ici.extent = { width, height, 1 };
	ici.mipLevels = 1;
	ici.arrayLayers = 1;
	ici.samples = VK_SAMPLE_COUNT_1_BIT;
	ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	ici.tiling = VK_IMAGE_TILING_OPTIMAL;
	ici.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	ici.queueFamilyIndexCount = 0;
	ici.pQueueFamilyIndices = nullptr;
	ici.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	ici.flags = 0;

 	if (vkCreateImage(m_logicalDevice, &ici, nullptr, &depthImage) != VK_SUCCESS)
		return false;

	VkMemoryRequirements memoryRequirements = {};
	vkGetImageMemoryRequirements(m_logicalDevice, depthImage, &memoryRequirements);

	VkMemoryAllocateInfo iai = {};
	iai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	iai.allocationSize = memoryRequirements.size;
	iai.memoryTypeIndex = getMemoryTypeIndex(m_physicalDevice, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, memoryRequirements);

	VkDeviceMemory imageMemory = {};
	if (vkAllocateMemory(m_logicalDevice, &iai, nullptr, &imageMemory) != VK_SUCCESS)
		return false;

	if (vkBindImageMemory(m_logicalDevice, depthImage, imageMemory, 0) != VK_SUCCESS)
		return false;

	// Create primary targets.
	m_primaryTargets.resize(imageCount);
	for (uint32_t i = 0; i < imageCount; ++i)
	{
		m_primaryTargets[i] = new RenderTargetSetVk(
			m_physicalDevice,
			m_logicalDevice,
			0,
			0,
			0
		);
		if (!m_primaryTargets[i]->createPrimary(
			width,
			height,
			colorFormat,
			presentImages[i],
			VK_FORMAT_D24_UNORM_S8_UINT,
			depthImage
		))
			return false;
	}

	// Create descriptor pool.
	VkDescriptorPoolSize dps[4];
	dps[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	dps[0].descriptorCount = 1000;
	dps[1].type = VK_DESCRIPTOR_TYPE_SAMPLER;
	dps[1].descriptorCount = 1000;
	dps[2].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	dps[2].descriptorCount = 1000;
	dps[3].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	dps[3].descriptorCount = 1000;

	VkDescriptorPoolCreateInfo dpci = {};
	dpci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	dpci.pNext = nullptr;
	dpci.maxSets = 1024;
	dpci.poolSizeCount = sizeof_array(dps);
	dpci.pPoolSizes = dps;

	if (vkCreateDescriptorPool(m_logicalDevice, &dpci, nullptr, &m_descriptorPool) != VK_SUCCESS)
		return false;

	// Create synchronization primitives.
    VkFenceCreateInfo fci = {};
    fci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    vkCreateFence(m_logicalDevice, &fci, nullptr, &m_renderFence);

	VkSemaphoreCreateInfo sci = {};
	sci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    vkCreateSemaphore(m_logicalDevice, &sci, nullptr, &m_presentCompleteSemaphore);

	// Set default viewport.
	m_viewport.left = 0;
	m_viewport.top = 0;
	m_viewport.width = width;
	m_viewport.height = height;
	m_viewport.nearZ = 0.0f;
	m_viewport.farZ = 1.0f;

	// Check if debug marker extension is available.
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &extensionCount, nullptr);

	AlignedVector< VkExtensionProperties > extensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &extensionCount, extensions.ptr());

	m_haveDebugMarkers = false;
	//for (auto extension : extensions)
	//{
	//	if (strcmp(extension.extensionName, VK_EXT_DEBUG_MARKER_EXTENSION_NAME) == 0)
	//	{
	//		log::info << L"Found debug marker extension; debug markers enabled." << Endl;
	//		m_haveDebugMarkers = true;
	//		break;
	//	}
	//}

	// Create uniform buffer pool.
	m_uniformBufferPool = new UniformBufferPoolVk(m_physicalDevice, m_logicalDevice);
	return true;
}

void RenderViewVk::validateTargetState()
{
	if (!m_targetStateDirty)
		return;

	T_FATAL_ASSERT (!m_targetStateStack.empty());
	TargetState& ts = m_targetStateStack.back();

	// Prepare render target set as targets.
	if (!ts.rts->prepareAsTarget(
		m_graphicsCommandBuffer,
		ts.colorIndex,
		ts.clear,
		m_primaryTargets[m_currentImageIndex]->getDepthTargetVk(),
		
		// Out
		m_targetId,
		m_targetRenderPass
	))
		return;

	VkViewport vp = {};
	vp.x = ts.viewport.left;
	vp.y = ts.viewport.top;
	vp.width = ts.viewport.width;
	vp.height = ts.viewport.height;
	vp.minDepth = ts.viewport.nearZ;
	vp.maxDepth = ts.viewport.farZ;
	vkCmdSetViewport(m_graphicsCommandBuffer, 0, 1, &vp);

	ts.clear.mask = 0;
	m_targetStateDirty = false;
}

bool RenderViewVk::validatePipeline(VertexBufferVk* vb, ProgramVk* p, PrimitiveType pt)
{
	uint32_t primitiveId = (uint32_t)pt;
	uint32_t declHash = vb->getHash();
	uint32_t programHash = p->getHash();

	const auto key = std::make_tuple(primitiveId, m_targetId, declHash, programHash);

	VkPipeline pipeline = 0;

	auto it = m_pipelines.find(key);
	if (it != m_pipelines.end())
		pipeline = it->second;
	else
	{
		const RenderState& rs = p->getRenderState();

		VkViewport vp = {};
		vp.width = 1;
		vp.height = 1;
		vp.minDepth = 0.0f;
		vp.maxDepth = 1.0f;

		VkRect2D sc = {};
		sc.offset = { 0, 0 };
		sc.extent = { 65536, 65536 };

		VkPipelineViewportStateCreateInfo vsci = {};
		vsci.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		vsci.viewportCount = 1;
		vsci.pViewports = &vp;
		vsci.scissorCount = 1;
		vsci.pScissors = &sc;

		VkPipelineVertexInputStateCreateInfo visci = {};
		visci.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		visci.vertexBindingDescriptionCount = 1;
		visci.pVertexBindingDescriptions = &vb->getVkVertexInputBindingDescription();
		visci.vertexAttributeDescriptionCount = vb->getVkVertexInputAttributeDescriptions().size();
		visci.pVertexAttributeDescriptions = vb->getVkVertexInputAttributeDescriptions().c_ptr();

		VkPipelineShaderStageCreateInfo ssci[2] = {};
		ssci[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		ssci[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		ssci[0].module = p->getVertexVkShaderModule();
		ssci[0].pName = "main";
		ssci[0].pSpecializationInfo = nullptr;
		ssci[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		ssci[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		ssci[1].module = p->getFragmentVkShaderModule();
		ssci[1].pName = "main";
		ssci[1].pSpecializationInfo = nullptr;

		VkPipelineRasterizationStateCreateInfo rsci = {};
		rsci.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rsci.depthClampEnable = VK_FALSE;
		rsci.rasterizerDiscardEnable = VK_FALSE;
		rsci.polygonMode = rs.wireframe ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
		rsci.cullMode = c_cullMode[rs.cullMode];
		rsci.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rsci.depthBiasEnable = VK_FALSE;
		rsci.depthBiasConstantFactor = 0;
		rsci.depthBiasClamp = 0;
		rsci.depthBiasSlopeFactor = 0;
		rsci.lineWidth = 1;

		VkPipelineMultisampleStateCreateInfo mssci = {};
		mssci.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		mssci.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		mssci.sampleShadingEnable = VK_FALSE;
		mssci.minSampleShading = 0;
		mssci.pSampleMask = nullptr;
		mssci.alphaToCoverageEnable = rs.alphaToCoverageEnable ? VK_TRUE : VK_FALSE;
		mssci.alphaToOneEnable = VK_FALSE;

		VkStencilOpState sops = {};
		sops.failOp = c_stencilOperations[rs.stencilFail];
		sops.passOp = c_stencilOperations[rs.stencilPass];
		sops.depthFailOp = c_stencilOperations[rs.stencilZFail];
		sops.compareOp = c_compareOperations[rs.stencilFunction];
		sops.compareMask = rs.stencilMask;
		sops.writeMask = rs.stencilMask;
		sops.reference = rs.stencilReference;

		VkPipelineDepthStencilStateCreateInfo dssci = {};
		dssci.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		dssci.depthTestEnable = rs.depthEnable ? VK_TRUE : VK_FALSE;
		dssci.depthWriteEnable = rs.depthWriteEnable ? VK_TRUE : VK_FALSE;
		dssci.depthCompareOp = rs.depthEnable ? c_compareOperations[rs.depthFunction] : VK_COMPARE_OP_ALWAYS;
		dssci.depthBoundsTestEnable = VK_FALSE;
		dssci.stencilTestEnable = rs.stencilEnable ? VK_TRUE : VK_FALSE;
		dssci.front = sops;
		dssci.back = sops;
		dssci.minDepthBounds = 0;
		dssci.maxDepthBounds = 0;

		AlignedVector< VkPipelineColorBlendAttachmentState > blendAttachments;

		for (uint32_t i = 0; i < 4; ++i)
		{
			auto& cbas = blendAttachments.push_back();
			cbas.blendEnable = rs.blendEnable ? VK_TRUE : VK_FALSE;
			cbas.srcColorBlendFactor = c_blendFactors[rs.blendColorSource];
			cbas.dstColorBlendFactor = c_blendFactors[rs.blendColorDestination];
			cbas.colorBlendOp = c_blendOperations[rs.blendColorOperation];
			cbas.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			cbas.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			cbas.alphaBlendOp = VK_BLEND_OP_ADD;
			cbas.colorWriteMask = rs.colorWriteMask;
		}

		VkPipelineColorBlendStateCreateInfo cbsci = {};
		cbsci.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		cbsci.logicOpEnable = VK_FALSE;
		cbsci.logicOp = VK_LOGIC_OP_CLEAR;
		cbsci.attachmentCount = (uint32_t)blendAttachments.size();
		cbsci.pAttachments = blendAttachments.c_ptr();
		cbsci.blendConstants[0] = 0.0;
		cbsci.blendConstants[1] = 0.0;
		cbsci.blendConstants[2] = 0.0;
		cbsci.blendConstants[3] = 0.0;

		VkDynamicState ds[1] = { VK_DYNAMIC_STATE_VIEWPORT };
		VkPipelineDynamicStateCreateInfo dsci = {};
		dsci.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dsci.dynamicStateCount = sizeof_array(ds);
		dsci.pDynamicStates = ds;

		VkPipelineInputAssemblyStateCreateInfo iasci = {};
		iasci.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		iasci.topology = c_primitiveTopology[pt];
		iasci.primitiveRestartEnable = VK_FALSE;

		VkGraphicsPipelineCreateInfo gpci = {};
		gpci.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		gpci.stageCount = 2;
		gpci.pStages = ssci;
		gpci.pVertexInputState = &visci;
		gpci.pInputAssemblyState = &iasci;
		gpci.pTessellationState = nullptr;
		gpci.pViewportState = &vsci;
		gpci.pRasterizationState = &rsci;
		gpci.pMultisampleState = &mssci;
		gpci.pDepthStencilState = &dssci;
		gpci.pColorBlendState = &cbsci;
		gpci.pDynamicState = &dsci;
		gpci.layout = p->getPipelineLayout();
		gpci.renderPass = m_targetRenderPass;
		gpci.subpass = 0;
		gpci.basePipelineHandle = 0;
		gpci.basePipelineIndex = 0;

		if (vkCreateGraphicsPipelines(m_logicalDevice, VK_NULL_HANDLE, 1, &gpci, nullptr, &pipeline) != VK_SUCCESS)
		{
			log::error << L"Unable to create Vulkan graphics pipeline." << Endl;
			return false;
		}

		m_pipelines[key] = pipeline;
	}

	if (!pipeline)
		return false;

	vkCmdBindPipeline(m_graphicsCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	return true;
}

#if defined(_WIN32)
bool RenderViewVk::windowListenerEvent(Window* window, UINT message, WPARAM wParam, LPARAM lParam, LRESULT& outResult)
{
	if (message == WM_CLOSE)
	{
		RenderEvent evt;
		evt.type = ReClose;
		m_eventQueue.push_back(evt);
	}
	else if (message == WM_SIZE)
	{
		// Remove all pending resize events.
		m_eventQueue.remove_if(RenderEventTypePred(ReResize));

		// Push new resize event if not matching current size.
		int32_t width = LOWORD(lParam);
		int32_t height = HIWORD(lParam);

		if (width <= 0 || height <= 0)
			return false;

		//DXGI_SWAP_CHAIN_DESC dxscd;
		//std::memset(&dxscd, 0, sizeof(dxscd));

		//if (m_dxgiSwapChain)
		//	m_dxgiSwapChain->GetDesc(&dxscd);

		//if (m_dxgiSwapChain == 0 || width != dxscd.BufferDesc.Width || height != dxscd.BufferDesc.Height)
		//{
		//	RenderEvent evt;
		//	evt.type = ReResize;
		//	evt.resize.width = width;
		//	evt.resize.height = height;
		//	m_eventQueue.push_back(evt);
		//}
	}
	else if (message == WM_SIZING)
	{
		RECT* rcWindowSize = (RECT*)lParam;

		int32_t width = rcWindowSize->right - rcWindowSize->left;
		int32_t height = rcWindowSize->bottom - rcWindowSize->top;

		if (width < 320)
			width = 320;
		if (height < 200)
			height = 200;

		if (wParam == WMSZ_RIGHT || wParam == WMSZ_TOPRIGHT || wParam == WMSZ_BOTTOMRIGHT)
			rcWindowSize->right = rcWindowSize->left + width;
		if (wParam == WMSZ_LEFT || wParam == WMSZ_TOPLEFT || wParam == WMSZ_BOTTOMLEFT)
			rcWindowSize->left = rcWindowSize->right - width;

		if (wParam == WMSZ_BOTTOM || wParam == WMSZ_BOTTOMLEFT || wParam == WMSZ_BOTTOMRIGHT)
			rcWindowSize->bottom = rcWindowSize->top + height;
		if (wParam == WMSZ_TOP || wParam == WMSZ_TOPLEFT || wParam == WMSZ_TOPRIGHT)
			rcWindowSize->top = rcWindowSize->bottom - height;

		outResult = TRUE;
	}
	else if (message == WM_SYSKEYDOWN)
	{
		if (wParam == VK_RETURN && (lParam & (1 << 29)) != 0)
		{
			RenderEvent evt;
			evt.type = ReToggleFullScreen;
			m_eventQueue.push_back(evt);
		}
	}
	else if (message == WM_KEYDOWN)
	{
		if (wParam == VK_RETURN && (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0)
		{
			RenderEvent evt;
			evt.type = ReToggleFullScreen;
			m_eventQueue.push_back(evt);
		}
	}
	else if (message == WM_SETCURSOR)
	{
		//if (!m_cursorVisible)
		//	SetCursor(NULL);
		//else
			return false;
	}
	else
		return false;

	return true;
}
#endif

	}
}
