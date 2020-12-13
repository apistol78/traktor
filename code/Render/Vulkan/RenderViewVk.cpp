#include <cstring>
#include "Core/Containers/StaticVector.h"
#include "Core/Log/Log.h"
#include "Core/Misc/AutoPtr.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Timer/Profiler.h"
#include "Render/Vulkan/ApiLoader.h"
#include "Render/Vulkan/CommandBufferPool.h"
#include "Render/Vulkan/CubeTextureVk.h"
#include "Render/Vulkan/IndexBufferVk.h"
#include "Render/Vulkan/ProgramVk.h"
#include "Render/Vulkan/Queue.h"
#include "Render/Vulkan/RenderTargetDepthVk.h"
#include "Render/Vulkan/RenderTargetVk.h"
#include "Render/Vulkan/RenderTargetSetVk.h"
#include "Render/Vulkan/RenderViewVk.h"
#include "Render/Vulkan/SimpleTextureVk.h"
#include "Render/Vulkan/UniformBufferPoolVk.h"
#include "Render/Vulkan/UtilitiesVk.h"
#include "Render/Vulkan/VertexBufferVk.h"

#if defined(__MAC__)
#	include "Render/Vulkan/macOS/Metal.h"
#elif defined(__IOS__)
#	include "Render/Vulkan/iOS/Utilities.h"
#endif

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

bool presentationModeSupported(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkPresentModeKHR presentationMode)
{
	uint32_t presentModeCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, 0);
	AutoArrayPtr< VkPresentModeKHR > presentModes(new VkPresentModeKHR[presentModeCount]);
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes.ptr());
	for (uint32_t i = 0; i < presentModeCount; ++i)
	{
		if (presentModes[i] == presentationMode)
			return true;
	}
	return false;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderViewVk", RenderViewVk, IRenderView)

RenderViewVk::RenderViewVk(
	VkInstance instance,
	VkPhysicalDevice physicalDevice,
	VkDevice logicalDevice,
	VmaAllocator allocator,
	Queue* graphicsQueue,
	Queue* computeQueue
)
:	m_instance(instance)
,	m_physicalDevice(physicalDevice)
,	m_logicalDevice(logicalDevice)
,	m_allocator(allocator)
,	m_graphicsQueue(graphicsQueue)
,	m_computeQueue(computeQueue)
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
#	if defined(_WIN32)
	m_window->addListener(this);
#	endif
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

	if (!create(desc.displayMode.width, desc.displayMode.height, desc.waitVBlanks))
		return false;

	return true;	
}

bool RenderViewVk::create(const RenderViewEmbeddedDesc& desc)
{
	VkResult result;
	int32_t width = 64;
	int32_t height = 64;

	// Create renderable surface.
#if defined(_WIN32)
    VkWin32SurfaceCreateInfoKHR sci = {};
    sci.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    sci.hinstance = GetModuleHandle(nullptr);
    sci.hwnd = (HWND)desc.syswin.hWnd;
    if ((result = vkCreateWin32SurfaceKHR(m_instance, &sci, nullptr, &m_surface)) != VK_SUCCESS)
	{
		log::error << L"Failed to create Vulkan; unable to create Win32 renderable surface (" << getHumanResult(result) << L")." << Endl;
		return false;
	}

	RECT rc;
	GetClientRect((HWND)desc.syswin.hWnd, &rc);
	width = (int32_t)(rc.right - rc.left);
	height = (int32_t)(rc.bottom - rc.top);

#elif defined(__LINUX__)
	VkXlibSurfaceCreateInfoKHR sci = {};
	sci.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
	sci.dpy = (::Display*)desc.syswin.display;
	sci.window = desc.syswin.window;
    if ((result = vkCreateXlibSurfaceKHR(m_instance, &sci, nullptr, &m_surface)) != VK_SUCCESS)
	{
		log::error << L"Failed to create Vulkan; unable to create X11 renderable surface (" << getHumanResult(result) << L")." << Endl;
		return false;
	}
#elif defined(__ANDROID__)
	VkAndroidSurfaceCreateInfoKHR sci = {};
	sci.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
	sci.flags = 0;
	sci.window = *desc.syswin.window;
	if ((result = vkCreateAndroidSurfaceKHR(m_instance, &sci, nullptr, &m_surface)) != VK_SUCCESS)
	{
		log::error << L"Failed to create Vulkan; unable to create Android renderable surface (" << getHumanResult(result) << L")." << Endl;
		return false;
	}

	width = ANativeWindow_getWidth(sci.window);
	height = ANativeWindow_getHeight(sci.window);
#elif defined(__MAC__)

	// Attach Metal layer to provided view.
	attachMetalLayer(desc.syswin.view);

	VkMetalSurfaceCreateInfoEXT  sci = {};
	sci.sType = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT;
	sci.pLayer = getMetalLayer(desc.syswin.view);
    if ((result = vkCreateMetalSurfaceEXT(m_instance, &sci, nullptr, &m_surface)) != VK_SUCCESS)
	{
		log::error << L"Failed to create Vulkan; unable to create macOS renderable surface (" << getHumanResult(result) << L")." << Endl;
		return false;
	}
#elif defined(__IOS__)
	VkIOSSurfaceCreateInfoMVK sci = {};
	sci.sType = VK_STRUCTURE_TYPE_IOS_SURFACE_CREATE_INFO_MVK;
	sci.pView = desc.syswin.view;
    if ((result = vkCreateIOSSurfaceMVK(m_instance, &sci, nullptr, &m_surface)) != VK_SUCCESS)
	{
		log::error << L"Failed to create Vulkan; unable to create iOS renderable surface (" << getHumanResult(result) << L")." << Endl;
		return false;
	}

	width = getViewWidth(desc.syswin.view);
	height = getViewHeight(desc.syswin.view);
#endif

	if (!create(width, height, desc.waitVBlanks))
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
	// Assume device is idle when lost.
	if (!m_lost)
		vkDeviceWaitIdle(m_logicalDevice);

	m_lost = true;

	// Ensure event queue doesn't contain stale events.
	m_eventQueue.clear();

	// Destroy frame resources.
	for (auto& frame : m_frames)
	{
		frame.primaryTarget->destroy();
		vkDestroyFence(m_logicalDevice, frame.inFlightFence, nullptr);
		vkDestroySemaphore(m_logicalDevice, frame.renderFinishedSemaphore, nullptr);
		vkDestroyDescriptorPool(m_logicalDevice, frame.descriptorPool, nullptr);
		m_computeCommandPool->release(frame.computeCommandBuffer);
		m_graphicsCommandPool->release(frame.graphicsCommandBuffer);
	}
	m_frames.clear();

	if (m_queryPool != 0)
	{
		vkDestroyQueryPool(m_logicalDevice, m_queryPool, nullptr);
		m_queryPool = 0;
	}

	if (m_imageAvailableSemaphore != 0)
	{
		vkDestroySemaphore(m_logicalDevice, m_imageAvailableSemaphore, nullptr);
		m_imageAvailableSemaphore = 0;
	}

	// Destroy pipelines.
	for (auto& pipeline : m_pipelines)
		vkDestroyPipeline(m_logicalDevice, pipeline.second.pipeline, nullptr);
	m_pipelines.clear();

	// Destroy previous swap chain.
	if (m_swapChain != 0)
	{
		vkDestroySwapchainKHR(m_logicalDevice, m_swapChain, 0);	
		m_swapChain = 0;
	}

	// Free command pools.
	m_computeCommandPool = nullptr;
	m_graphicsCommandPool = nullptr;
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

	if (desc.fullscreen)
		m_window->setFullScreenStyle(desc.displayMode.width, desc.displayMode.height);
	else
		m_window->setWindowedStyle(desc.displayMode.width, desc.displayMode.height);

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
	close();

#if defined(_WIN32)
	if (m_window)
		m_window->setWindowedStyle(width, height);
#endif

	if (create(width, height, m_vblanks))
		return true;
	else
		return false;
}

int RenderViewVk::getWidth() const
{
	if (!m_frames.empty())
		return m_frames.front().primaryTarget->getWidth();
	else
		return 0;
}

int RenderViewVk::getHeight() const
{
	if (!m_frames.empty())
		return m_frames.front().primaryTarget->getHeight();
	else
		return 0;
}

bool RenderViewVk::isActive() const
{
#if defined(_WIN32) || defined(__ANDROID__) || defined(__MAC__) || defined(__IOS__)
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
#elif defined(__ANDROID__) || defined(__MAC__) || defined(__IOS__)
	return true;
#else
	return m_window->isFullScreen();
#endif
}

void RenderViewVk::showCursor()
{
	m_cursorVisible = true;
}

void RenderViewVk::hideCursor()
{
	m_cursorVisible = false;
}

bool RenderViewVk::isCursorVisible() const
{
	return m_cursorVisible;
}

bool RenderViewVk::setGamma(float gamma)
{
	return false;
}

void RenderViewVk::setViewport(const Viewport& viewport)
{
	T_ASSERT(viewport.width > 0);
	T_ASSERT(viewport.height > 0);

	const auto& frame = m_frames[m_currentImageIndex];

	VkViewport vp = {};
	vp.x = (float)viewport.left;
	vp.y = (float)viewport.top;
	vp.width = (float)viewport.width;
	vp.height = (float)viewport.height;
	vp.minDepth = viewport.nearZ;
	vp.maxDepth = viewport.farZ;
	vkCmdSetViewport(frame.graphicsCommandBuffer, 0, 1, &vp);
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

bool RenderViewVk::beginFrame()
{
	const uint64_t timeOut = 5 * 60 * 1000ull * 1000ull * 1000ull;
	VkResult result;

	// Might reach here with a non-created instance, pending reset, so
	// we need to make sure we have an instance first.
	if (m_lost || m_frames.empty())
		return false;

	// Do this first so we remember, count number of frames.
	m_counter++;

	// Get next target from swap chain.
    vkAcquireNextImageKHR(
		m_logicalDevice,
		m_swapChain,
		timeOut,
		m_imageAvailableSemaphore,
		VK_NULL_HANDLE,
		&m_currentImageIndex
	);
	if (m_currentImageIndex >= m_frames.size())
		return false;

	auto& frame = m_frames[m_currentImageIndex];

    result = vkWaitForFences(m_logicalDevice, 1, &frame.inFlightFence, VK_TRUE, timeOut);
	if (result != VK_SUCCESS)
	{
		log::warning << L"Vulkan error reported, \"" << getHumanResult(result) << L"\"; need to reset renderer (2)." << Endl;
		
		// Issue an event in order to reset view.
		RenderEvent evt;
		evt.type = ReLost;
		m_eventQueue.push_back(evt);
		m_lost = true;
		return false;
	}

	// Reset descriptor pool.
	result = vkResetDescriptorPool(m_logicalDevice, frame.descriptorPool, 0);
	if (result != VK_SUCCESS)
		return false;

	// Begin recording command buffer.
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	result = vkBeginCommandBuffer(frame.graphicsCommandBuffer, &beginInfo);
	if (result != VK_SUCCESS)
		return false;
	
	// Reset time queries.
	const int32_t querySegmentCount = (int32_t)(m_frames.size() * 2);
	const int32_t queryFrom = (m_counter % querySegmentCount) * 1024;
	vkCmdResetQueryPool(frame.graphicsCommandBuffer, m_queryPool, queryFrom, 1024);
	m_nextQueryIndex = queryFrom;

	// Reset misc counters.
	m_passCount = 0;
	m_drawCalls = 0;
	m_primitiveCount = 0;
	return true;
}

void RenderViewVk::endFrame()
{
	auto& frame = m_frames[m_currentImageIndex];
	VkResult result;

	// Prepare primary color for presentation.
	frame.primaryTarget->getColorTargetVk(0)->prepareForPresentation(frame.graphicsCommandBuffer);

	// End recording command buffer.
	result = vkEndCommandBuffer(frame.graphicsCommandBuffer);
	if (result != VK_SUCCESS)
	{
		log::warning << L"Vulkan error reported, \"" << getHumanResult(result) << L"\"; need to reset renderer (1)." << Endl;
		
		// Issue an event in order to reset view.
		RenderEvent evt;
		evt.type = ReLost;
		m_eventQueue.push_back(evt);
		m_lost = true;
		return;
	}

	vkResetFences(m_logicalDevice, 1, &frame.inFlightFence);

	// Submit commands to graphics queue.
	VkPipelineStageFlags waitStageMash = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	VkSubmitInfo si = {};
	si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	si.waitSemaphoreCount = 1;
	si.pWaitSemaphores = &m_imageAvailableSemaphore;
	si.pWaitDstStageMask = &waitStageMash;
	si.commandBufferCount = 1;
	si.pCommandBuffers = &frame.graphicsCommandBuffer;
	si.signalSemaphoreCount = 1;
	si.pSignalSemaphores = &frame.renderFinishedSemaphore;
	m_graphicsQueue->submit(si, frame.inFlightFence);

	// Release unused pipelines.
	for (auto it = m_pipelines.begin(); it != m_pipelines.end(); )
	{
		if ((m_counter - it->second.lastAcquired) >= 16)	// Pipelines are kept for X number of frames before getting collected.
		{
			vkDestroyPipeline(m_logicalDevice, it->second.pipeline, nullptr);
			it = m_pipelines.erase(it);
		}
		else
			it++;
	}

	// Collect, or cycle, released buffers.
	m_uniformBufferPool->collect();
}

void RenderViewVk::present()
{
	auto& frame = m_frames[m_currentImageIndex];
	VkResult result;

	// Queue presentation of current primary target.
    VkPresentInfoKHR pi = {};
    pi.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    pi.swapchainCount = 1;
    pi.pSwapchains = &m_swapChain;
    pi.pImageIndices = &m_currentImageIndex;
    pi.waitSemaphoreCount = 1;
    pi.pWaitSemaphores = &frame.renderFinishedSemaphore;
    pi.pResults = nullptr;

    result = vkQueuePresentKHR(m_presentQueue, &pi);
	if (result != VK_SUCCESS)
	{
		log::warning << L"Vulkan error reported, \"" << getHumanResult(result) << L"\"; need to reset renderer (3)." << Endl;

		// Issue an event in order to reset view.
		RenderEvent evt;
		evt.type = ReLost;
		m_eventQueue.push_back(evt);
		m_lost = true;
		return;
	}
}

bool RenderViewVk::beginPass(const Clear* clear)
{
	T_FATAL_ASSERT(m_targetRenderPass == 0);

	if (m_lost)
		return false;

	auto& frame = m_frames[m_currentImageIndex];

	Clear cl = {};
	if (clear)
		cl = *clear;

	m_targetSet = frame.primaryTarget;
	m_targetColorIndex = 0;

	// Prepare render target set as targets.
	if (!m_targetSet->prepareAsTarget(
		frame.graphicsCommandBuffer,
		m_targetColorIndex,
		cl,
		TfColor | TfDepth,
		TfColor,
		frame.primaryTarget->getDepthTargetVk(),
		
		// Out
		m_targetId,
		m_targetRenderPass,
		m_targetFrameBuffer
	))
		return false;

	// Transform clear values.
	StaticVector< VkClearValue, 1+1 > clearValues;
	{
		auto& cv = clearValues.push_back();
		cl.colors[0].storeUnaligned(cv.color.float32);
	}
	{
		auto& cv = clearValues.push_back();
		cv.depthStencil.depth = cl.depth;
		cv.depthStencil.stencil = cl.stencil;
	}

	// Begin render pass.
	VkRenderPassBeginInfo rpbi = {};
	rpbi.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rpbi.renderPass = m_targetRenderPass;
	rpbi.framebuffer = m_targetFrameBuffer;
	rpbi.renderArea.offset.x = 0;
	rpbi.renderArea.offset.y = 0;
	rpbi.renderArea.extent.width = m_targetSet->getWidth();
	rpbi.renderArea.extent.height = m_targetSet->getHeight();
	rpbi.clearValueCount = (uint32_t)clearValues.size();
	rpbi.pClearValues = clearValues.c_ptr();
	vkCmdBeginRenderPass(frame.graphicsCommandBuffer, &rpbi,  VK_SUBPASS_CONTENTS_INLINE);

	// Set viewport.
	VkViewport vp = {};
	vp.x = 0.0f;
	vp.y = 0.0f;
	vp.width = (float)m_targetSet->getWidth();
	vp.height = (float)m_targetSet->getHeight();
	vp.minDepth = 0.0f;
	vp.maxDepth = 1.0f;
	vkCmdSetViewport(frame.graphicsCommandBuffer, 0, 1, &vp);

	m_passCount++;
	return true;
}

bool RenderViewVk::beginPass(IRenderTargetSet* renderTargetSet, const Clear* clear, uint32_t load, uint32_t store)
{
	T_FATAL_ASSERT(m_targetRenderPass == 0);

	if (m_lost)
		return false;

	auto& frame = m_frames[m_currentImageIndex];

	Clear cl = {};
	if (clear)
		cl = *clear;

	m_targetSet = mandatory_non_null_type_cast< RenderTargetSetVk* >(renderTargetSet);
	m_targetColorIndex = -1;

	// Prepare render target set as targets.
	if (!m_targetSet->prepareAsTarget(
		frame.graphicsCommandBuffer,
		m_targetColorIndex,
		cl,
		load,
		store,
		frame.primaryTarget->getDepthTargetVk(),
		
		// Out
		m_targetId,
		m_targetRenderPass,
		m_targetFrameBuffer
	))
		return false;

	// Transform clear values.
	StaticVector< VkClearValue, 8+1 > clearValues;
	if (m_targetColorIndex >= 0)
	{
		auto& cv = clearValues.push_back();
		cl.colors[0].storeUnaligned(cv.color.float32);
	}
	else
	{
		for (int32_t i = 0; i < (int32_t)m_targetSet->getColorTargetCount(); ++i)
		{
			auto& cv = clearValues.push_back();
			cl.colors[i].storeUnaligned(cv.color.float32);
		}
	}
	if (m_targetSet->getDepthTargetVk() || m_targetSet->usingPrimaryDepthStencil())
	{
		auto& cv = clearValues.push_back();
		cv.depthStencil.depth = cl.depth;
		cv.depthStencil.stencil = cl.stencil;
	}

	// Begin render pass.
	VkRenderPassBeginInfo rpbi = {};
	rpbi.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rpbi.renderPass = m_targetRenderPass;
	rpbi.framebuffer = m_targetFrameBuffer;
	rpbi.renderArea.offset.x = 0;
	rpbi.renderArea.offset.y = 0;
	rpbi.renderArea.extent.width = m_targetSet->getWidth();
	rpbi.renderArea.extent.height = m_targetSet->getHeight();
	rpbi.clearValueCount = (uint32_t)clearValues.size();
	rpbi.pClearValues = clearValues.c_ptr();
	vkCmdBeginRenderPass(frame.graphicsCommandBuffer, &rpbi,  VK_SUBPASS_CONTENTS_INLINE);

	// Set viewport.
	VkViewport vp = {};
	vp.x = 0.0f;
	vp.y = 0.0f;
	vp.width = (float)m_targetSet->getWidth();
	vp.height = (float)m_targetSet->getHeight();
	vp.minDepth = 0.0f;
	vp.maxDepth = 1.0f;
	vkCmdSetViewport(frame.graphicsCommandBuffer, 0, 1, &vp);

	m_passCount++;
	return true;
}

bool RenderViewVk::beginPass(IRenderTargetSet* renderTargetSet, int32_t renderTarget, const Clear* clear, uint32_t load, uint32_t store)
{
	T_FATAL_ASSERT(m_targetRenderPass == 0);

	if (m_lost)
		return false;

	auto& frame = m_frames[m_currentImageIndex];

	Clear cl = {};
	if (clear)
		cl = *clear;

	m_targetSet = mandatory_non_null_type_cast< RenderTargetSetVk* >(renderTargetSet);
	m_targetColorIndex = renderTarget;

	// Prepare render target set as targets.
	if (!m_targetSet->prepareAsTarget(
		frame.graphicsCommandBuffer,
		m_targetColorIndex,
		cl,
		load,
		store,
		frame.primaryTarget->getDepthTargetVk(),
		
		// Out
		m_targetId,
		m_targetRenderPass,
		m_targetFrameBuffer
	))
		return false;

	// Transform clear values.
	StaticVector< VkClearValue, 8+1 > clearValues;
	if (m_targetColorIndex >= 0)
	{
		auto& cv = clearValues.push_back();
		cl.colors[0].storeUnaligned(cv.color.float32);
	}
	else
	{
		for (int32_t i = 0; i < (int32_t)m_targetSet->getColorTargetCount(); ++i)
		{
			auto& cv = clearValues.push_back();
			cl.colors[i].storeUnaligned(cv.color.float32);
		}
	}
	if (m_targetSet->getDepthTargetVk() || m_targetSet->usingPrimaryDepthStencil())
	{
		auto& cv = clearValues.push_back();
		cv.depthStencil.depth = cl.depth;
		cv.depthStencil.stencil = cl.stencil;
	}

	// Begin render pass.
	VkRenderPassBeginInfo rpbi = {};
	rpbi.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rpbi.renderPass = m_targetRenderPass;
	rpbi.framebuffer = m_targetFrameBuffer;
	rpbi.renderArea.offset.x = 0;
	rpbi.renderArea.offset.y = 0;
	rpbi.renderArea.extent.width = m_targetSet->getWidth();
	rpbi.renderArea.extent.height = m_targetSet->getHeight();
	rpbi.clearValueCount = (uint32_t)clearValues.size();
	rpbi.pClearValues = clearValues.c_ptr();
	vkCmdBeginRenderPass(frame.graphicsCommandBuffer, &rpbi,  VK_SUBPASS_CONTENTS_INLINE);

	// Set viewport.
	VkViewport vp = {};
	vp.x = 0.0f;
	vp.y = 0.0f;
	vp.width = (float)m_targetSet->getWidth();
	vp.height = (float)m_targetSet->getHeight();
	vp.minDepth = 0.0f;
	vp.maxDepth = 1.0f;
	vkCmdSetViewport(frame.graphicsCommandBuffer, 0, 1, &vp);

	m_passCount++;
	return true;
}

void RenderViewVk::endPass()
{
	auto& frame = m_frames[m_currentImageIndex];

	// Close current render pass.
	vkCmdEndRenderPass(frame.graphicsCommandBuffer);

	// Transition target to texture if necessary.
	if (m_targetSet != frame.primaryTarget)
	{
		m_targetSet->prepareAsTexture(
			frame.graphicsCommandBuffer,
			m_targetColorIndex
		);
	}

	m_targetSet = nullptr;
	m_targetId = 0;
	m_targetRenderPass = 0;
	m_targetFrameBuffer = 0;
}

void RenderViewVk::draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives)
{
	draw(vertexBuffer, indexBuffer, program, primitives, 1);
}

void RenderViewVk::draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives, uint32_t instanceCount)
{
	VertexBufferVk* vb = mandatory_non_null_type_cast< VertexBufferVk* >(vertexBuffer);
	ProgramVk* p = mandatory_non_null_type_cast< ProgramVk* >(program);

	auto& frame = m_frames[m_currentImageIndex];

	validatePipeline(vb, p, primitives.type);

	float targetSize[] =
	{
		(float)m_targetSet->getWidth(),
		(float)m_targetSet->getHeight()
	};
	p->validateGraphics(frame.descriptorPool, frame.graphicsCommandBuffer, m_uniformBufferPool, targetSize);

	const uint32_t c_primitiveMul[] = { 1, 0, 2, 2, 3 };
	uint32_t vertexCount = primitives.count * c_primitiveMul[primitives.type];

	VkBuffer vbb = vb->getVkBuffer();
	VkDeviceSize offsets = {};
	vkCmdBindVertexBuffers(frame.graphicsCommandBuffer, 0, 1, &vbb, &offsets);

	if (indexBuffer && primitives.indexed)
	{
		IndexBufferVk* ib = mandatory_non_null_type_cast< IndexBufferVk* >(indexBuffer);
		VkBuffer ibb = ib->getVkBuffer();

		VkDeviceSize offset = {};
		vkCmdBindIndexBuffer(
			frame.graphicsCommandBuffer,
			ibb,
			offset,
			(ib->getIndexType() == ItUInt16) ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32
		);

		vkCmdDrawIndexed(
			frame.graphicsCommandBuffer,
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
			frame.graphicsCommandBuffer,
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
	auto& frame = m_frames[m_currentImageIndex];

	ProgramVk* p = mandatory_non_null_type_cast< ProgramVk* >(program);
	p->validateCompute(frame.descriptorPool, frame.computeCommandBuffer, m_uniformBufferPool);
	vkCmdDispatch(frame.computeCommandBuffer, workSize[0], workSize[1], workSize[2]);
}

bool RenderViewVk::copy(ITexture* destinationTexture, const Region& destinationRegion, ITexture* sourceTexture, const Region& sourceRegion)
{
	auto& frame = m_frames[m_currentImageIndex];

	VkImage sourceImage = 0;
	VkImageLayout sourceImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	VkImage destinationImage = 0;
	VkImageLayout destinationImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	VkImageCopy region = {};
	region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.srcSubresource.mipLevel = sourceRegion.mip;
	region.srcSubresource.baseArrayLayer = 0;
	region.srcSubresource.layerCount = 1;
	region.srcOffset = { 0, 0, 0 };
	region.srcOffset.x = sourceRegion.x;
	region.srcOffset.y = sourceRegion.y;
	region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.dstSubresource.mipLevel = destinationRegion.mip;
	region.dstSubresource.baseArrayLayer = 0;
	region.dstSubresource.layerCount = 1;
	region.dstOffset = { 0, 0, 0 };
	region.dstOffset.x = destinationRegion.x;
	region.dstOffset.y = destinationRegion.y;
	region.extent = { 0, 0, 1 };
	region.extent.width = sourceRegion.width;
	region.extent.height = sourceRegion.height;

	if (auto sourceRenderTarget = dynamic_type_cast< RenderTargetVk* >(sourceTexture))
	{
		sourceImage = sourceRenderTarget->getVkImage();
		sourceImageLayout = sourceRenderTarget->getVkImageLayout();
	}
	else if (auto sourceSimpleTexture = dynamic_type_cast< SimpleTextureVk* >(sourceTexture))
	{
		sourceImage = sourceSimpleTexture->getVkImage();
		sourceImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	}
	else if (auto sourceCubeTexture = dynamic_type_cast< CubeTextureVk* >(sourceTexture))
	{
		sourceImage = sourceCubeTexture->getVkImage();
		sourceImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		region.srcSubresource.baseArrayLayer = sourceRegion.z;
	}
	else
		return false;

	if (auto destinationRenderTarget = dynamic_type_cast< RenderTargetVk* >(destinationTexture))
	{
		destinationImage = destinationRenderTarget->getVkImage();
		destinationImageLayout = destinationRenderTarget->getVkImageLayout();
	}
	else if (auto destinationSimpleTexture = dynamic_type_cast< SimpleTextureVk* >(destinationTexture))
	{
		destinationImage = destinationSimpleTexture->getVkImage();
		destinationImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	}
	else if (auto destinationCubeTexture = dynamic_type_cast< CubeTextureVk* >(destinationTexture))
	{
		destinationImage = destinationCubeTexture->getVkImage();
		destinationImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		region.dstSubresource.baseArrayLayer = destinationRegion.z;
	}
	else
		return false;

	// Source texture layout.
	{
		VkImageMemoryBarrier imb = {};
		imb.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imb.oldLayout = sourceImageLayout;
		imb.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		imb.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imb.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imb.image = sourceImage;
		imb.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imb.subresourceRange.baseMipLevel = sourceRegion.mip;
		imb.subresourceRange.levelCount = 1;
		imb.subresourceRange.baseArrayLayer = region.srcSubresource.baseArrayLayer;
		imb.subresourceRange.layerCount = 1;
		imb.srcAccessMask = 0;
		imb.dstAccessMask = 0;

		vkCmdPipelineBarrier(
			frame.graphicsCommandBuffer,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &imb
		);
	}

	// Destination texture layout.
	{
		VkImageMemoryBarrier imb = {};
		imb.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imb.oldLayout = destinationImageLayout;
		imb.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imb.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imb.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imb.image = destinationImage;
		imb.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imb.subresourceRange.baseMipLevel = destinationRegion.mip;
		imb.subresourceRange.levelCount = 1;
		imb.subresourceRange.baseArrayLayer = region.dstSubresource.baseArrayLayer;
		imb.subresourceRange.layerCount = 1;
		imb.srcAccessMask = 0;
		imb.dstAccessMask = 0;

		vkCmdPipelineBarrier(
			frame.graphicsCommandBuffer,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &imb
		);
	}

	// Perform texture image copy.
	vkCmdCopyImage(
		frame.graphicsCommandBuffer,
		sourceImage,
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		destinationImage,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&region
	);

	// Source texture layout.
	{
		VkImageMemoryBarrier imb = {};
		imb.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imb.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		imb.newLayout = sourceImageLayout;
		imb.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imb.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imb.image = sourceImage;
		imb.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imb.subresourceRange.baseMipLevel = sourceRegion.mip;
		imb.subresourceRange.levelCount = 1;
		imb.subresourceRange.baseArrayLayer = region.srcSubresource.baseArrayLayer;
		imb.subresourceRange.layerCount = 1;
		imb.srcAccessMask = 0;
		imb.dstAccessMask = 0;

		vkCmdPipelineBarrier(
			frame.graphicsCommandBuffer,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &imb
		);
	}

	// Destination texture layout.
	{
		VkImageMemoryBarrier imb = {};
		imb.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imb.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imb.newLayout = destinationImageLayout;
		imb.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imb.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imb.image = destinationImage;
		imb.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imb.subresourceRange.baseMipLevel = destinationRegion.mip;
		imb.subresourceRange.levelCount = 1;
		imb.subresourceRange.baseArrayLayer = region.dstSubresource.baseArrayLayer;
		imb.subresourceRange.layerCount = 1;
		imb.srcAccessMask = 0;
		imb.dstAccessMask = 0;

		vkCmdPipelineBarrier(
			frame.graphicsCommandBuffer,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &imb
		);
	}

	return true;
}

int32_t RenderViewVk::beginTimeQuery()
{
	auto& frame = m_frames[m_currentImageIndex];
	const int32_t query = m_nextQueryIndex;
	vkCmdWriteTimestamp(frame.graphicsCommandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, m_queryPool, query + 0);
	m_nextQueryIndex += 2;
	return query;
}

void RenderViewVk::endTimeQuery(int32_t query)
{
	auto& frame = m_frames[m_currentImageIndex];
	vkCmdWriteTimestamp(frame.graphicsCommandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, m_queryPool, query + 1);
}

bool RenderViewVk::getTimeQuery(int32_t query, bool wait, double& outStart, double& outEnd) const
{
	uint32_t flags = VK_QUERY_RESULT_64_BIT;
	if (wait)
		flags |= VK_QUERY_RESULT_WAIT_BIT;

	uint64_t stamps[2] = { 0, 0 };

	VkResult result = vkGetQueryPoolResults(m_logicalDevice, m_queryPool, query, 2, 2 * sizeof(uint64_t), stamps, sizeof(uint64_t), flags);
	if (result != VK_SUCCESS)
		return false;

	const double c_divend = 1000000000.0;
	outStart = (double)stamps[0] / c_divend;
	outEnd = (double)stamps[1] / c_divend;
	return true;
}

void RenderViewVk::pushMarker(const char* const marker)
{
#if !defined(__ANDROID__) && !defined(__IOS__)
	if (m_haveDebugMarkers)
	{
		auto& frame = m_frames[m_currentImageIndex];

		VkDebugUtilsLabelEXT dul = {};
		dul.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
		dul.pLabelName = marker;
		vkCmdBeginDebugUtilsLabelEXT(frame.graphicsCommandBuffer, &dul);
	}
#endif
}

void RenderViewVk::popMarker()
{
#if !defined(__ANDROID__) && !defined(__IOS__)
	if (m_haveDebugMarkers)
	{
		auto& frame = m_frames[m_currentImageIndex];
		vkCmdEndDebugUtilsLabelEXT(frame.graphicsCommandBuffer);
	}
#endif
}

void RenderViewVk::getStatistics(RenderViewStatistics& outStatistics) const
{
	outStatistics.passCount = m_passCount;
	outStatistics.drawCalls = m_drawCalls;
	outStatistics.primitiveCount = m_primitiveCount;
}

bool RenderViewVk::create(uint32_t width, uint32_t height, int32_t vblanks)
{
	// In case we fail to create make sure we're lost.
	m_lost = true;

	// Clamp surface size to physical device limits.
	VkSurfaceCapabilitiesKHR surfaceCapabilities = {};
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice, m_surface, &surfaceCapabilities);

	width = std::max(surfaceCapabilities.minImageExtent.width, width);
	width = std::min(surfaceCapabilities.maxImageExtent.width, width);
	height = std::max(surfaceCapabilities.minImageExtent.height, height);
	height = std::min(surfaceCapabilities.maxImageExtent.height, height);

	// Do not fail if requested size, assume it will get reset later.
	if (width == 0 || height == 0)
		return true;

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

	m_graphicsCommandPool = CommandBufferPool::create(m_logicalDevice, m_graphicsQueue);
	m_computeCommandPool = CommandBufferPool::create(m_logicalDevice, m_computeQueue);

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
	VkPresentModeKHR presentationMode = VK_PRESENT_MODE_FIFO_KHR;
#if defined(__ANDROID__) || defined(__IOS__) || defined(__LINUX__)
	if (presentationModeSupported(m_physicalDevice, m_surface, VK_PRESENT_MODE_MAILBOX_KHR))
		presentationMode = VK_PRESENT_MODE_MAILBOX_KHR;
#endif
	if (vblanks <= 0)
	{
		if (presentationModeSupported(m_physicalDevice, m_surface, VK_PRESENT_MODE_IMMEDIATE_KHR))
			presentationMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
	}

	if (presentationMode == VK_PRESENT_MODE_FIFO_KHR)
		log::debug << L"Using FIFO presentation mode." << Endl;
	else if (presentationMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
		log::debug << L"Using IMMEDIATE presentation mode." << Endl;
	else if (presentationMode == VK_PRESENT_MODE_MAILBOX_KHR)
		log::debug << L"Using MAILBOX presentation mode." << Endl;

	m_vblanks = vblanks;

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
	scci.clipped = VK_TRUE;

	uint32_t queueFamilyIndices[] = { m_graphicsQueue->getQueueIndex(), m_presentQueueIndex };
	if (m_graphicsQueue->getQueueIndex() != m_presentQueueIndex)
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
	vkGetSwapchainImagesKHR(m_logicalDevice, m_swapChain, &imageCount, nullptr);

	AlignedVector< VkImage > presentImages(imageCount);
	vkGetSwapchainImagesKHR(m_logicalDevice, m_swapChain, &imageCount, presentImages.ptr());

	log::debug << L"Using " << imageCount << L" images in swap chain; requested " << desiredImageCount << L" image(s)." << Endl;

	// Create primary depth image.
	VkImage depthImage = 0;

	VkImageCreateInfo ici = {};
	ici.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	ici.imageType = VK_IMAGE_TYPE_2D;
#if defined(__IOS__)
	ici.format = VK_FORMAT_D16_UNORM_S8_UINT;
#else
	ici.format = VK_FORMAT_D24_UNORM_S8_UINT;
#endif
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

	VkSemaphoreCreateInfo sci = {};
	sci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	vkCreateSemaphore(m_logicalDevice, &sci, nullptr, &m_imageAvailableSemaphore);

	// Create time query pool.
	VkQueryPoolCreateInfo qpci = {};
	qpci.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
	qpci.pNext = nullptr;
	qpci.queryType = VK_QUERY_TYPE_TIMESTAMP;
	qpci.queryCount = imageCount * 2 * 1024;
	if (vkCreateQueryPool(m_logicalDevice, &qpci, nullptr, &m_queryPool) != VK_SUCCESS)
		return false;

	// Create frame resources.
	m_frames.resize(imageCount);
	for (uint32_t i = 0; i < imageCount; ++i)
	{
		auto& frame = m_frames[i];

		frame.graphicsCommandBuffer = m_graphicsCommandPool->acquire();
		frame.computeCommandBuffer = m_computeCommandPool->acquire();

		VkDescriptorPoolSize dps[4];
		dps[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		dps[0].descriptorCount = 40000;
		dps[1].type = VK_DESCRIPTOR_TYPE_SAMPLER;
		dps[1].descriptorCount = 40000;
		dps[2].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		dps[2].descriptorCount = 40000;
		dps[3].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		dps[3].descriptorCount = 4000;

		VkDescriptorPoolCreateInfo dpci = {};
		dpci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		dpci.pNext = nullptr;
		dpci.maxSets = 4096;
		dpci.poolSizeCount = sizeof_array(dps);
		dpci.pPoolSizes = dps;
		if (vkCreateDescriptorPool(m_logicalDevice, &dpci, nullptr, &frame.descriptorPool) != VK_SUCCESS)
			return false;

		VkSemaphoreCreateInfo sci = {};
		sci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		vkCreateSemaphore(m_logicalDevice, &sci, nullptr, &frame.renderFinishedSemaphore);

		VkFenceCreateInfo fci = {};
		fci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fci.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		vkCreateFence(m_logicalDevice, &fci, nullptr, &frame.inFlightFence);	

		frame.primaryTarget = new RenderTargetSetVk(
			m_physicalDevice,
			m_logicalDevice,
			0,
			m_graphicsQueue,
			m_graphicsCommandPool
		);
		if (!frame.primaryTarget->createPrimary(
			width,
			height,
			colorFormat,
			presentImages[i],
			ici.format,
			depthImage,
			str(L"Primary %d", i).c_str()
		))
			return false;
	}

	// Check if debug marker extension is available.
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &extensionCount, nullptr);

	AlignedVector< VkExtensionProperties > extensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &extensionCount, extensions.ptr());

	m_haveDebugMarkers = false;
#if !defined(__ANDROID__) && !defined(__IOS__)
	for (auto extension : extensions)
	{
		if (std::strcmp(extension.extensionName, VK_EXT_DEBUG_MARKER_EXTENSION_NAME) == 0)
		{
			log::info << L"Found debug marker extension; debug markers enabled." << Endl;
			m_haveDebugMarkers = true;
			break;
		}
	}
#endif

	// Create uniform buffer pool.
	m_uniformBufferPool = new UniformBufferPoolVk(m_logicalDevice, m_allocator);

	m_nextQueryIndex = 0;
	m_lost = false;
	return true;
}

bool RenderViewVk::validatePipeline(VertexBufferVk* vb, ProgramVk* p, PrimitiveType pt)
{
	auto& frame = m_frames[m_currentImageIndex];
	
	uint32_t primitiveId = (uint32_t)pt;
	uint32_t declHash = vb->getHash();
	uint32_t shaderHash = p->getShaderHash();

	const auto key = std::make_tuple(primitiveId, m_targetId, declHash, shaderHash);

	VkPipeline pipeline = 0;

	auto it = m_pipelines.find(key);
	if (it != m_pipelines.end())
	{
		it->second.lastAcquired = m_counter;
		pipeline = it->second.pipeline;
	}
	else
	{
		const RenderState& rs = p->getRenderState();

		uint32_t colorAttachmentCount = m_targetSet->getColorTargetCount();
		if (m_targetColorIndex >= 0)
			colorAttachmentCount = 1;

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
		visci.vertexAttributeDescriptionCount = (uint32_t)vb->getVkVertexInputAttributeDescriptions().size();
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

		for (uint32_t i = 0; i < colorAttachmentCount; ++i)
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

		VkDynamicState ds[2] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_STENCIL_REFERENCE };
		VkPipelineDynamicStateCreateInfo dsci = {};
		dsci.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dsci.dynamicStateCount = rs.stencilEnable ? 2 : 1;
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

		VkResult result = vkCreateGraphicsPipelines(m_logicalDevice, VK_NULL_HANDLE, 1, &gpci, nullptr, &pipeline);
		if (result != VK_SUCCESS)
		{
			log::error << L"Unable to create Vulkan graphics pipeline (" << getHumanResult(result) << L")." << Endl;
			return false;
		}

		m_pipelines[key] = { m_counter, pipeline };
	}

	if (!pipeline)
		return false;

	vkCmdBindPipeline(frame.graphicsCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
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

		if (width != getWidth() || height != getHeight())
		{
			RenderEvent evt;
			evt.type = ReResize;
			evt.resize.width = width;
			evt.resize.height = height;
			m_eventQueue.push_back(evt);
		}
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
		if (!m_cursorVisible)
			SetCursor(NULL);
		else
			return false;
	}
	else
		return false;

	return true;
}
#endif

	}
}
