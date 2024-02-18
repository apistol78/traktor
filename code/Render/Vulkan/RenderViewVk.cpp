/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#define NOMINMAX
#include <algorithm>
#include <cstring>
#include "Core/Containers/StaticVector.h"
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Misc/AutoPtr.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Core/Timer/Profiler.h"
#include "Render/Vulkan/BufferViewVk.h"
#include "Render/Vulkan/ProgramVk.h"
#include "Render/Vulkan/RenderTargetDepthVk.h"
#include "Render/Vulkan/RenderTargetVk.h"
#include "Render/Vulkan/RenderTargetSetVk.h"
#include "Render/Vulkan/RenderViewVk.h"
#include "Render/Vulkan/TextureVk.h"
#include "Render/Vulkan/VertexLayoutVk.h"
#include "Render/Vulkan/Private/ApiLoader.h"
#include "Render/Vulkan/Private/CommandBuffer.h"
#include "Render/Vulkan/Private/Context.h"
#include "Render/Vulkan/Private/Image.h"
#include "Render/Vulkan/Private/Queue.h"
#include "Render/Vulkan/Private/RenderPassCache.h"
#include "Render/Vulkan/Private/Utilities.h"

#if defined(__MAC__)
#	include "Render/Vulkan/macOS/Metal.h"
#elif defined(__IOS__)
#	include "Render/Vulkan/iOS/Utilities.h"
#endif

namespace traktor::render
{
	namespace
	{

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

VkPipelineStageFlagBits convertStage(Stage st)
{
	uint32_t ps = 0;
	if ((st & Stage::Vertex) == Stage::Vertex)
		ps |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
	if ((st & Stage::Fragment) == Stage::Fragment)
		ps |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	if ((st & Stage::Compute) == Stage::Compute)
		ps |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
	if ((st & Stage::Indirect) == Stage::Indirect)
		ps |= VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;
	return (VkPipelineStageFlagBits)ps;
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderViewVk", RenderViewVk, IRenderView)

RenderViewVk::RenderViewVk(
	Context* context,
	VkInstance instance
)
:	m_context(context)
,	m_instance(instance)
{
	m_context->incrementViews();
}

RenderViewVk::~RenderViewVk()
{
	close();
	m_context->decrementViews();
}

bool RenderViewVk::create(const RenderViewDefaultDesc& desc)
{
#if defined(_WIN32) || defined(__LINUX__) || defined(__RPI__) || defined(__MAC__)
	// Create render window.
	m_window = new Window();
	if (!m_window->create(desc.display, desc.displayMode.width, desc.displayMode.height))
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
#elif defined(__LINUX__) || defined(__RPI__)
	VkXlibSurfaceCreateInfoKHR sci = {};
	sci.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
	sci.dpy = m_window->getDisplay();
	sci.window = m_window->getWindow();
    if (vkCreateXlibSurfaceKHR(m_instance, &sci, nullptr, &m_surface) != VK_SUCCESS)
	{
		log::error << L"Failed to create Vulkan; unable to create X11 renderable surface." << Endl;
		return false;
	}
#elif defined(__MAC__)

	// Attach Metal layer to provided view.
	attachMetalLayer(m_window->getView());

	VkMetalSurfaceCreateInfoEXT  sci = {};
	sci.sType = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT;
	sci.pLayer = getMetalLayer(m_window->getView());
    if (vkCreateMetalSurfaceEXT(m_instance, &sci, nullptr, &m_surface) != VK_SUCCESS)
	{
		log::error << L"Failed to create Vulkan; unable to create macOS renderable surface." << Endl;
		return false;
	}
#endif

	if (!create(desc.displayMode.width, desc.displayMode.height, desc.multiSample, desc.multiSampleShading, desc.waitVBlanks))
		return false;

	return true;	
}

bool RenderViewVk::create(const RenderViewEmbeddedDesc& desc)
{
	VkResult result;
	int32_t width = 0;
	int32_t height = 0;

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

	// Get size of surfce.
	RECT rc;
	GetClientRect((HWND)desc.syswin.hWnd, &rc);
	width = (int32_t)(rc.right - rc.left);
	height = (int32_t)(rc.bottom - rc.top);

#elif defined(__LINUX__) || defined(__RPI__)
	VkXlibSurfaceCreateInfoKHR sci = {};
	sci.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
	sci.dpy = (::Display*)desc.syswin.display;
	sci.window = desc.syswin.window;
    if ((result = vkCreateXlibSurfaceKHR(m_instance, &sci, nullptr, &m_surface)) != VK_SUCCESS)
	{
		log::error << L"Failed to create Vulkan; unable to create X11 renderable surface (" << getHumanResult(result) << L")." << Endl;
		return false;
	}

	// Get size of surface.
	VkSurfaceCapabilitiesKHR sc;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_context->getPhysicalDevice(), m_surface, &sc);
	width = sc.currentExtent.width;
	height = sc.currentExtent.height;

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

	// Get size of surface.
	const int32_t resolutionDenom = 2;
	width = ANativeWindow_getWidth(sci.window) / resolutionDenom;
	height = ANativeWindow_getHeight(sci.window) / resolutionDenom;
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
    
    // Get size of surface.
    VkSurfaceCapabilitiesKHR sc;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_context->getPhysicalDevice(), m_surface, &sc);
    width = sc.currentExtent.width;
    height = sc.currentExtent.height;
#elif defined(__IOS__)
	VkIOSSurfaceCreateInfoMVK sci = {};
	sci.sType = VK_STRUCTURE_TYPE_IOS_SURFACE_CREATE_INFO_MVK;
	sci.pView = desc.syswin.view;
    if ((result = vkCreateIOSSurfaceMVK(m_instance, &sci, nullptr, &m_surface)) != VK_SUCCESS)
	{
		log::error << L"Failed to create Vulkan; unable to create iOS renderable surface (" << getHumanResult(result) << L")." << Endl;
		return false;
	}

	// Get size of surfce.
	width = getViewWidth(desc.syswin.view);
	height = getViewHeight(desc.syswin.view);
#endif

	if (!create(width, height, desc.multiSample, desc.multiSampleShading, desc.waitVBlanks))
		return false;

	return true;
}

bool RenderViewVk::nextEvent(RenderEvent& outEvent)
{
#if defined(_WIN32)
	if (m_window)
	{
		MSG msg;
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
#elif defined(__LINUX__) || defined(__RPI__) || defined(__MAC__)
	if (m_window)
	{
		if (m_window->update(outEvent))
			return true;
	}
#endif

	if (m_eventQueue.empty())
		return false;

	outEvent = m_eventQueue.front();
	m_eventQueue.pop_front();
	return true;
}

void RenderViewVk::close()
{
	vkDeviceWaitIdle(m_context->getLogicalDevice());

	// Ensure any pending cleanups are performed before closing render view.
	m_context->savePipelineCache();
	m_context->performCleanup();
	m_lost = true;

	// Ensure event queue doesn't contain stale events.
	m_eventQueue.clear();

	// Destroy frame resources.
	for (auto& frame : m_frames)
	{
		if (frame.graphicsCommandBuffer)
			frame.graphicsCommandBuffer->wait();

		frame.primaryTarget->destroy();
		vkDestroySemaphore(m_context->getLogicalDevice(), frame.renderFinishedSemaphore, nullptr);

		frame.graphicsCommandBuffer = nullptr;
	}
	m_frames.clear();

	// More pending cleanups since frames own render targets.
	m_context->performCleanup();

#if !defined(__ANDROID__) && !defined(__IOS__)
	if (m_queryPool != 0)
	{
		vkDestroyQueryPool(m_context->getLogicalDevice(), m_queryPool, nullptr);
		m_queryPool = 0;
	}
#endif

	if (m_imageAvailableSemaphore != 0)
	{
		vkDestroySemaphore(m_context->getLogicalDevice(), m_imageAvailableSemaphore, nullptr);
		m_imageAvailableSemaphore = 0;
	}

	// Destroy pipelines.
	for (auto& pipeline : m_pipelines)
		vkDestroyPipeline(m_context->getLogicalDevice(), pipeline.second.pipeline, nullptr);
	m_pipelines.clear();

	// Destroy previous swap chain.
	if (m_swapChain != 0)
	{
		vkDestroySwapchainKHR(m_context->getLogicalDevice(), m_swapChain, nullptr);	
		m_swapChain = 0;
	}

	// Destroy surface.
	if (m_surface != 0)
	{
		vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
		m_surface = 0;
	}

	m_presentQueue = nullptr;
	m_counter = -1;
}

bool RenderViewVk::reset(const RenderViewDefaultDesc& desc)
{
#if defined(_WIN32) || defined(__LINUX__) || defined(__RPI__)
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
	vkDeviceWaitIdle(m_context->getLogicalDevice());

	// Ensure any pending cleanups are performed before closing render view.
	m_context->performCleanup();
	m_lost = true;

	// Ensure event queue doesn't contain stale events.
	m_eventQueue.clear();

	// Destroy frame resources.
	for (auto& frame : m_frames)
	{
		if (frame.graphicsCommandBuffer)
			frame.graphicsCommandBuffer->externalSynced();

		frame.primaryTarget->destroy();
		vkDestroySemaphore(m_context->getLogicalDevice(), frame.renderFinishedSemaphore, nullptr);
	}
	m_frames.clear();

#if !defined(__ANDROID__) && !defined(__IOS__)
	if (m_queryPool != 0)
	{
		vkDestroyQueryPool(m_context->getLogicalDevice(), m_queryPool, nullptr);
		m_queryPool = 0;
	}
#endif

	if (m_imageAvailableSemaphore != 0)
	{
		vkDestroySemaphore(m_context->getLogicalDevice(), m_imageAvailableSemaphore, nullptr);
		m_imageAvailableSemaphore = 0;
	}

	// Destroy pipelines.
	for (auto& pipeline : m_pipelines)
		vkDestroyPipeline(m_context->getLogicalDevice(), pipeline.second.pipeline, nullptr);
	m_pipelines.clear();

	// More pending cleanups since frames own render targets.
	m_context->performCleanup();
	m_counter = -1;

	if (create(width, height, m_multiSample, m_multiSampleShading, m_vblanks))
		return true;
	else
		return false;
}

uint32_t RenderViewVk::getDisplay() const
{
#if defined(_WIN32)
	return m_window->getDisplay();
#else
	return 0;
#endif
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
#if defined(__ANDROID__) || defined(__IOS__)
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
#elif defined(__ANDROID__) || defined(__IOS__)
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
	vkCmdSetViewport(*frame.graphicsCommandBuffer, 0, 1, &vp);
}

SystemWindow RenderViewVk::getSystemWindow()
{
#if defined(_WIN32)
	return SystemWindow(*m_window);
#elif defined(__LINUX__) || defined(__RPI__)
	return SystemWindow(m_window->getDisplay(), m_window->getWindow());
#elif defined(__MAC__)
	return SystemWindow(m_window->getView());
#else
	return SystemWindow();
#endif
}

bool RenderViewVk::beginFrame()
{
	// Might reach here with a non-created instance, pending reset, so
	// we need to make sure we have an instance first.
	if (m_lost)
		return false;

	// Do this first so we remember, count number of frames.
	m_counter++;

	// Update VMA once each frame. 
	vmaSetCurrentFrameIndex(m_context->getAllocator(), m_counter);

	// Get next target from swap chain.
	T_PROFILER_BEGIN(L"vkAcquireNextImageKHR");
    vkAcquireNextImageKHR(
		m_context->getLogicalDevice(),
		m_swapChain,
		UINT64_MAX,
		m_imageAvailableSemaphore,
		VK_NULL_HANDLE,
		&m_currentImageIndex
	);
	T_PROFILER_END();
	if (m_currentImageIndex >= m_frames.size())
		return false;

	auto& frame = m_frames[m_currentImageIndex];
	frame.markers.clear();

	// Reset command buffers.
	// \hack Lazy create since we don't know about rendering thread until beginFrame
	// is called... This assumes no other thread will perform rendering during the
	// life time of the render view.
	T_PROFILER_BEGIN(L"Wait graphics queue");
	if (frame.graphicsCommandBuffer)
	{
		// Ensure command buffer has been consumed by GPU.
		if (!frame.graphicsCommandBuffer->wait())
		{
			// Issue an event in order to reset view.
			RenderEvent evt;
			evt.type = RenderEventType::Lost;
			m_eventQueue.push_back(evt);
			m_lost = true;
			return false;
		}

		if (!frame.graphicsCommandBuffer->reset())
			return false;
	}
	else
	{
		frame.graphicsCommandBuffer = m_context->getGraphicsQueue()->acquireCommandBuffer(T_FILE_LINE_W);
		if (!frame.graphicsCommandBuffer)
			return false;
	}
	T_PROFILER_END();

#if !defined(__ANDROID__) && !defined(__IOS__)
	// Reset time queries.
	const int32_t querySegmentCount = (int32_t)(m_frames.size() * 2);
	const int32_t queryFrom = (m_counter % querySegmentCount) * 1024;
	vkCmdResetQueryPool(*frame.graphicsCommandBuffer, m_queryPool, queryFrom, 1024);
	m_nextQueryIndex = queryFrom;
	m_lastQueryIndex = queryFrom + 1024;
#endif

	// Reset misc counters.
	m_passCount = 0;
	m_drawCalls = 0;
	m_primitiveCount = 0;
	return true;
}

void RenderViewVk::endFrame()
{
	T_PROFILER_SCOPE(L"RenderViewVk::endFrame");

	auto& frame = m_frames[m_currentImageIndex];

	frame.boundPipeline = 0;
	frame.boundComputePipeline = 0;
	frame.boundIndexBuffer = BufferViewVk();
	frame.boundVertexBuffer = BufferViewVk();

	// Prepare primary color for presentation.
	frame.primaryTarget->getColorTargetVk(0)->prepareForPresentation(frame.graphicsCommandBuffer);

	frame.graphicsCommandBuffer->submit(
		m_imageAvailableSemaphore,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		frame.renderFinishedSemaphore
	);

#if 0
	// Release unused pipelines.
	for (auto it = m_pipelines.begin(); it != m_pipelines.end(); )
	{
		if ((m_counter - it->second.lastAcquired) >= 16)	// Pipelines are kept for X number of frames before getting collected.
		{
			log::debug << L"Destroying unused pipeline." << Endl;
			vkDestroyPipeline(m_context->getLogicalDevice(), it->second.pipeline, nullptr);
			it = m_pipelines.erase(it);
		}
		else
			it++;
	}
#endif
}

void RenderViewVk::present()
{
	T_PROFILER_SCOPE(L"RenderViewVk::present");

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
	if ((result = m_presentQueue->present(pi)) != VK_SUCCESS)
	{
		log::warning << L"Vulkan error reported, \"" << getHumanResult(result) << L"\"; need to reset renderer (3)." << Endl;

		// Issue an event in order to reset view.
		RenderEvent evt;
		evt.type = RenderEventType::Lost;
		m_eventQueue.push_back(evt);
		m_lost = true;
		return;
	}

	// Cleanup destroyed resources.
	m_context->performCleanup();

	// Recycle uniform buffers.
	m_context->recycle();
}

bool RenderViewVk::beginPass(const Clear* clear, uint32_t load, uint32_t store)
{
	const auto& frame = m_frames[m_currentImageIndex];
	return beginPass(
		frame.primaryTarget,
		0,
		clear,
		load,
		store
	);
}

bool RenderViewVk::beginPass(IRenderTargetSet* renderTargetSet, const Clear* clear, uint32_t load, uint32_t store)
{
	T_FATAL_ASSERT(m_targetRenderPass == 0);

	if (m_lost)
		return false;

	const auto& frame = m_frames[m_currentImageIndex];
	const Clear cl = clear ? *clear : Clear{};

	m_targetSet = mandatory_non_null_type_cast< RenderTargetSetVk* >(renderTargetSet);
	m_targetColorIndex = -1;

	// Get render pass.
	RenderPassCache::Specification rp = {};
	rp.msaaSampleCount = m_targetSet->getVkSampleCount();
	rp.clear = cl.mask;
	rp.load = (uint8_t)load;
	rp.store = (uint8_t)store;
	for (uint32_t i = 0; i < m_targetSet->getColorTargetCount(); ++i)
		rp.colorTargetFormats[i] = m_targetSet->getColorTargetVk(i)->getVkFormat();
	if (m_targetSet->getDepthTargetVk())
		rp.depthTargetFormat = m_targetSet->getDepthTargetVk()->getVkFormat();
	else if (m_targetSet->usingPrimaryDepthStencil())
		rp.depthTargetFormat = frame.primaryTarget->getDepthTargetVk()->getVkFormat();
	if (!m_renderPassCache->get(rp, m_targetRenderPass))
		return false;

	// Store hash of render pass specification for pipeline cache.
	m_targetRenderPassHash = rp.hash();

	// Prepare render target set as targets.
	if (!m_targetSet->prepareAsTarget(
		frame.graphicsCommandBuffer,
		m_targetColorIndex,
		m_targetRenderPass,
		frame.primaryTarget->getDepthTargetVk(),
		
		// Out
		m_targetFrameBuffer
	))
		return false;

	// Transform clear values.
	StaticVector< VkClearValue, 16+1 > clearValues;
	for (int32_t i = 0; i < (int32_t)m_targetSet->getColorTargetCount(); ++i)
	{
		auto& cv = clearValues.push_back();
		cl.colors[i].storeUnaligned(cv.color.float32);
		if (m_targetSet->needResolve())
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
	vkCmdBeginRenderPass(*frame.graphicsCommandBuffer, &rpbi,  VK_SUBPASS_CONTENTS_INLINE);

	// Set viewport.
	VkViewport vp = {};
	vp.x = 0.0f;
	vp.y = 0.0f;
	vp.width = (float)m_targetSet->getWidth();
	vp.height = (float)m_targetSet->getHeight();
	vp.minDepth = 0.0f;
	vp.maxDepth = 1.0f;
	vkCmdSetViewport(*frame.graphicsCommandBuffer, 0, 1, &vp);

	m_passCount++;
	return true;
}

bool RenderViewVk::beginPass(IRenderTargetSet* renderTargetSet, int32_t renderTarget, const Clear* clear, uint32_t load, uint32_t store)
{
	T_FATAL_ASSERT(m_targetRenderPass == 0);

	if (m_lost)
		return false;

	const auto& frame = m_frames[m_currentImageIndex];
	const Clear cl = clear ? *clear : Clear{};

	m_targetSet = mandatory_non_null_type_cast< RenderTargetSetVk* >(renderTargetSet);
	m_targetColorIndex = renderTarget;

	// Get render pass.
	RenderPassCache::Specification rp = {};
	rp.msaaSampleCount = m_targetSet->getVkSampleCount();
	rp.clear = cl.mask;
	rp.load = (uint8_t)load;
	rp.store = (uint8_t)store;
	if (m_targetColorIndex >= 0)
		rp.colorTargetFormats[0] = m_targetSet->getColorTargetVk(m_targetColorIndex)->getVkFormat();
	else
	{
		for (uint32_t i = 0; i < m_targetSet->getColorTargetCount(); ++i)
			rp.colorTargetFormats[i] = m_targetSet->getColorTargetVk(i)->getVkFormat();
	}
	if (m_targetSet->getDepthTargetVk())
		rp.depthTargetFormat = m_targetSet->getDepthTargetVk()->getVkFormat();
	else if (m_targetSet->usingPrimaryDepthStencil())
		rp.depthTargetFormat = frame.primaryTarget->getDepthTargetVk()->getVkFormat();
	if (!m_renderPassCache->get(rp, m_targetRenderPass))
		return false;

	// Store hash of render pass specification for pipeline cache.
	m_targetRenderPassHash = rp.hash();

	// Prepare render target set as targets.
	if (!m_targetSet->prepareAsTarget(
		frame.graphicsCommandBuffer,
		m_targetColorIndex,
		m_targetRenderPass,
		frame.primaryTarget->getDepthTargetVk(),
		
		// Out
		m_targetFrameBuffer
	))
		return false;

	// Transform clear values.
	StaticVector< VkClearValue, 16+1 > clearValues;
	if (m_targetColorIndex >= 0)
	{
		auto& cv = clearValues.push_back();
		cl.colors[0].storeUnaligned(cv.color.float32);
		if (m_targetSet->needResolve())
		{
			auto& cv = clearValues.push_back();
			cl.colors[0].storeUnaligned(cv.color.float32);
		}
	}
	else
	{
		for (int32_t i = 0; i < (int32_t)m_targetSet->getColorTargetCount(); ++i)
		{
			auto& cv = clearValues.push_back();
			cl.colors[i].storeUnaligned(cv.color.float32);
			if (m_targetSet->needResolve())
			{
				auto& cv = clearValues.push_back();
				cl.colors[i].storeUnaligned(cv.color.float32);
			}
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
	vkCmdBeginRenderPass(*frame.graphicsCommandBuffer, &rpbi,  VK_SUBPASS_CONTENTS_INLINE);

	// Set viewport.
	VkViewport vp = {};
	vp.x = 0.0f;
	vp.y = 0.0f;
	vp.width = (float)m_targetSet->getWidth();
	vp.height = (float)m_targetSet->getHeight();
	vp.minDepth = 0.0f;
	vp.maxDepth = 1.0f;
	vkCmdSetViewport(*frame.graphicsCommandBuffer, 0, 1, &vp);

	m_passCount++;
	return true;
}

void RenderViewVk::endPass()
{
	const auto& frame = m_frames[m_currentImageIndex];

	// Close current render pass.
	vkCmdEndRenderPass(*frame.graphicsCommandBuffer);

	// Transition target to texture if necessary.
	if (m_targetSet != frame.primaryTarget)
	{
		m_targetSet->prepareAsTexture(
			frame.graphicsCommandBuffer,
			m_targetColorIndex
		);
	}

	m_targetSet = nullptr;
	m_targetRenderPass = 0;
	m_targetFrameBuffer = 0;
}

void RenderViewVk::draw(const IBufferView* vertexBuffer, const IVertexLayout* vertexLayout, const IBufferView* indexBuffer, IndexType indexType, IProgram* program, const Primitives& primitives, uint32_t instanceCount)
{
	const BufferViewVk* vbv = static_cast< const BufferViewVk* >(vertexBuffer);
	const VertexLayoutVk* vlv = static_cast< const VertexLayoutVk* >(vertexLayout);
	ProgramVk* p = static_cast< ProgramVk* >(program);

	auto& frame = m_frames[m_currentImageIndex];

	if (!validateGraphicsPipeline(vlv, p, primitives.type))
		return;

	const float targetSize[] = { (float)m_targetSet->getWidth(), (float)m_targetSet->getHeight() };
	if (!p->validate(frame.graphicsCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, targetSize))
		return;

	if (frame.boundVertexBuffer != *vbv)
	{
		const VkBuffer buffer = vbv->getVkBuffer();
		const VkDeviceSize offset = vbv->getVkBufferOffset();
		vkCmdBindVertexBuffers(*frame.graphicsCommandBuffer, 0, 1, &buffer, &offset);
		frame.boundVertexBuffer = *vbv;
	}

	if (indexBuffer && primitives.indexed)
	{
		const BufferViewVk* ibv = static_cast< const BufferViewVk* >(indexBuffer);
		if (frame.boundIndexBuffer != *ibv)
		{
			const VkBuffer buffer = ibv->getVkBuffer();
			const VkDeviceSize offset = ibv->getVkBufferOffset();
			vkCmdBindIndexBuffer(
				*frame.graphicsCommandBuffer,
				buffer,
				offset,
				(indexType == IndexType::UInt16) ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32
			);
			frame.boundIndexBuffer = *ibv;
		}

		vkCmdDrawIndexed(
			*frame.graphicsCommandBuffer,
			primitives.getVertexCount(),	// index count
			instanceCount,	// instance count
			primitives.offset,	// first index
			0,	// vertex offset
			0	// first instance id
		);
	}
	else
	{
		vkCmdDraw(
			*frame.graphicsCommandBuffer,
			primitives.getVertexCount(),   // vertex count
			instanceCount,   // instance count
			primitives.offset,   // first vertex
			0 // first instance
		);
	}

	m_drawCalls++;
	m_primitiveCount += primitives.count * instanceCount;
}

void RenderViewVk::drawIndirect(const IBufferView* vertexBuffer, const IVertexLayout* vertexLayout, const IBufferView* indexBuffer, IndexType indexType, IProgram* program, PrimitiveType primitiveType, const IBufferView* drawBuffer, uint32_t drawCount)
{
	const BufferViewVk* dbv = static_cast< const BufferViewVk* >(drawBuffer);
	const BufferViewVk* vbv = static_cast< const BufferViewVk* >(vertexBuffer);
	const VertexLayoutVk* vlv = static_cast< const VertexLayoutVk* >(vertexLayout);
	ProgramVk* p = static_cast< ProgramVk* >(program);

	auto& frame = m_frames[m_currentImageIndex];

	if (!validateGraphicsPipeline(vlv, p, primitiveType))
		return;

	const float targetSize[] = { (float)m_targetSet->getWidth(), (float)m_targetSet->getHeight() };
	if (!p->validate(frame.graphicsCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,  targetSize))
		return;

	if (frame.boundVertexBuffer != *vbv)
	{
		const VkBuffer buffer = vbv->getVkBuffer();
		const VkDeviceSize offset = vbv->getVkBufferOffset();
		vkCmdBindVertexBuffers(*frame.graphicsCommandBuffer, 0, 1, &buffer, &offset);
		frame.boundVertexBuffer = *vbv;
	}

	if (indexBuffer)
	{
		const BufferViewVk* ibv = static_cast< const BufferViewVk* >(indexBuffer);
		if (frame.boundIndexBuffer != *ibv)
		{
			const VkBuffer buffer = ibv->getVkBuffer();
			const VkDeviceSize offset = ibv->getVkBufferOffset();
			vkCmdBindIndexBuffer(
				*frame.graphicsCommandBuffer,
				buffer,
				offset,
				(indexType == IndexType::UInt16) ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32
			);
			frame.boundIndexBuffer = *ibv;
		}

		vkCmdDrawIndexedIndirect(
			*frame.graphicsCommandBuffer,
			dbv->getVkBuffer(),
			dbv->getVkBufferOffset(),
			drawCount,
			sizeof(VkDrawIndexedIndirectCommand)
		);
	}
	else
	{
		vkCmdDrawIndirect(
			*frame.graphicsCommandBuffer,
			dbv->getVkBuffer(),
			dbv->getVkBufferOffset(),
			drawCount,
			sizeof(VkDrawIndexedIndirectCommand)
		);
	}

	m_drawCalls++;
}

void RenderViewVk::compute(IProgram* program, const int32_t* workSize)
{
	ProgramVk* p = mandatory_non_null_type_cast< ProgramVk* >(program);
	const auto& frame = m_frames[m_currentImageIndex];

	if (!validateComputePipeline(p))
		return;

	if (!p->validate(frame.graphicsCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, nullptr))
		return;

	const int32_t* lwgs = p->getLocalWorkGroupSize();

	vkCmdDispatch(
		*frame.graphicsCommandBuffer,
		(workSize[0] + lwgs[0] - 1) / lwgs[0],
		(workSize[1] + lwgs[1] - 1) / lwgs[1],
		(workSize[2] + lwgs[2] - 1) / lwgs[2]
	);
}

void RenderViewVk::barrier(Stage from, Stage to)
{
	const auto& frame = m_frames[m_currentImageIndex];
	if (from == Stage::Compute && to == Stage::Indirect)
	{
		VkMemoryBarrier mb = {};
		mb.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
		mb.pNext = nullptr;
		mb.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
		mb.dstAccessMask = VK_ACCESS_INDIRECT_COMMAND_READ_BIT;

		vkCmdPipelineBarrier(
			*frame.graphicsCommandBuffer,
			convertStage(from),
			convertStage(to),
			0,
			1, &mb,
			0, nullptr,
			0, nullptr
		);
	}
	else if (from == Stage::Compute && to == Stage::Compute)
	{
		VkMemoryBarrier mb = {};
		mb.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
		mb.pNext = nullptr;
		mb.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
		mb.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(
			*frame.graphicsCommandBuffer,
			convertStage(from),
			convertStage(to),
			0,
			1, &mb,
			0, nullptr,
			0, nullptr
		);
	}
	else
	{
		// No memory access; only add an execution barrier.
		vkCmdPipelineBarrier(
			*frame.graphicsCommandBuffer,
			convertStage(from),
			convertStage(to),
			0,
			0, nullptr,
			0, nullptr,
			0, nullptr
		);
	}
}

bool RenderViewVk::copy(ITexture* destinationTexture, const Region& destinationRegion, ITexture* sourceTexture, const Region& sourceRegion)
{
	const auto& frame = m_frames[m_currentImageIndex];

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

	Image* sourceImage = nullptr;
	Image* destinationImage = nullptr;

	if (auto sourceRenderTarget = dynamic_type_cast< RenderTargetVk* >(sourceTexture))
		sourceImage = sourceRenderTarget->getImageResolved();
	else if (auto sourceTextureVk = dynamic_type_cast< TextureVk* >(sourceTexture))
	{
		sourceImage = sourceTextureVk->getImage();
		region.srcSubresource.baseArrayLayer = sourceRegion.z;
	}
	else
		return false;

	if (auto destinationRenderTarget = dynamic_type_cast< RenderTargetVk* >(destinationTexture))
		destinationImage = destinationRenderTarget->getImageResolved();
	else if (auto destinationTextureVk = dynamic_type_cast< TextureVk* >(destinationTexture))
	{
		destinationImage = destinationTextureVk->getImage();
		region.dstSubresource.baseArrayLayer = destinationRegion.z;
	}
	else
		return false;

	const VkImageLayout sourceImageLayout = sourceImage->getVkImageLayout(sourceRegion.mip, region.srcSubresource.baseArrayLayer);
	const VkImageLayout destinationImageLayout = destinationImage->getVkImageLayout(destinationRegion.mip, region.dstSubresource.baseArrayLayer);

	// Change image layouts for optimal transfer.
	if (!sourceImage->changeLayout(
		frame.graphicsCommandBuffer,
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		VK_IMAGE_ASPECT_COLOR_BIT,
		sourceRegion.mip,
		1,
		region.srcSubresource.baseArrayLayer,
		1
	))
		return false;

	if (!destinationImage->changeLayout(
		frame.graphicsCommandBuffer,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_ASPECT_COLOR_BIT,
		destinationRegion.mip,
		1,
		region.dstSubresource.baseArrayLayer,
		1
	))
		return false;

	// Perform texture image copy.
	vkCmdCopyImage(
		*frame.graphicsCommandBuffer,
		sourceImage->getVkImage(),
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		destinationImage->getVkImage(),
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&region
	);

	// Restore image layouts.
	if (!sourceImage->changeLayout(
		frame.graphicsCommandBuffer,
		sourceImageLayout,
		VK_IMAGE_ASPECT_COLOR_BIT,
		sourceRegion.mip,
		1,
		region.srcSubresource.baseArrayLayer,
		1
	))
		return false;

	if (!destinationImage->changeLayout(
		frame.graphicsCommandBuffer,
		destinationImageLayout,
		VK_IMAGE_ASPECT_COLOR_BIT,
		destinationRegion.mip,
		1,
		region.dstSubresource.baseArrayLayer,
		1
	))
		return false;

	return true;
}

int32_t RenderViewVk::beginTimeQuery()
{
#if !defined(__ANDROID__) && !defined(__IOS__)
	if (m_nextQueryIndex >= m_lastQueryIndex)
		return -1;

	auto& frame = m_frames[m_currentImageIndex];
	const int32_t query = m_nextQueryIndex;
	vkCmdWriteTimestamp(*frame.graphicsCommandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, m_queryPool, query + 0);
	m_nextQueryIndex += 2;

	return query;
#else
	return 0;
#endif
}

void RenderViewVk::endTimeQuery(int32_t query)
{
#if !defined(__ANDROID__) && !defined(__IOS__)
	auto& frame = m_frames[m_currentImageIndex];
	vkCmdWriteTimestamp(*frame.graphicsCommandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, m_queryPool, query + 1);
#endif
}

bool RenderViewVk::getTimeQuery(int32_t query, bool wait, double& outStart, double& outEnd) const
{
#if !defined(__ANDROID__) && !defined(__IOS__)
	uint32_t flags = VK_QUERY_RESULT_64_BIT;
	if (wait)
		flags |= VK_QUERY_RESULT_WAIT_BIT;

	uint64_t stamps[2] = { 0, 0 };

	VkResult result = vkGetQueryPoolResults(m_context->getLogicalDevice(), m_queryPool, query, 2, 2 * sizeof(uint64_t), stamps, sizeof(uint64_t), flags);
	if (result != VK_SUCCESS)
		return false;

	const double c_divend = 1000000000.0;
	outStart = (double)stamps[0] / c_divend;
	outEnd = (double)stamps[1] / c_divend;
	return true;
#else
	return false;
#endif
}

void RenderViewVk::pushMarker(const std::wstring& marker)
{
#if !defined(__ANDROID__) && !defined(__IOS__)
	if (m_haveDebugMarkers)
	{
		auto& frame = m_frames[m_currentImageIndex];
		if (!marker.empty())
		{
			frame.markers.push_back(wstombs(marker));

			VkDebugUtilsLabelEXT dul = {};
			dul.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
			dul.pLabelName = frame.markers.back().c_str();
			vkCmdBeginDebugUtilsLabelEXT(*frame.graphicsCommandBuffer, &dul);

			frame.markerStack.push_back(true);
		}
		else
			frame.markerStack.push_back(false);
	}
#endif
}

void RenderViewVk::popMarker()
{
#if !defined(__ANDROID__) && !defined(__IOS__)
	if (m_haveDebugMarkers)
	{
		auto& frame = m_frames[m_currentImageIndex];
		if (frame.markerStack.back())
			vkCmdEndDebugUtilsLabelEXT(*frame.graphicsCommandBuffer);
		frame.markerStack.pop_back();
	}
#endif
}

void RenderViewVk::getStatistics(RenderViewStatistics& outStatistics) const
{
	outStatistics.passCount = m_passCount;
	outStatistics.drawCalls = m_drawCalls;
	outStatistics.primitiveCount = m_primitiveCount;
}

bool RenderViewVk::create(uint32_t width, uint32_t height, uint32_t multiSample, float multiSampleShading, int32_t vblanks)
{
	log::debug << L"Vulkan; Render view create:" << Endl;
	log::debug << L"\twidth " << width << Endl;
	log::debug << L"\theight " << height << Endl;
	log::debug << L"\tmultiSample " << multiSample << Endl;
	log::debug << L"\tmultiSampleShading " << multiSampleShading << Endl;
	log::debug << L"\tvblanks " << vblanks << Endl;

	// In case we fail to create make sure we're lost.
	m_lost = true;
	m_multiSample = multiSample;
	m_multiSampleShading = multiSampleShading;
	m_vblanks = vblanks;

	// Do not fail if requested size, assume it will get reset later.
	if (width == 0 || height == 0)
	{
		log::debug << L"Vulkan: View size 0 * 0, wait for view to be reset." << Endl;
		return true;
	}

	// Clamp surface size to physical device limits.
	VkSurfaceCapabilitiesKHR surfaceCapabilities = {};
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_context->getPhysicalDevice(), m_surface, &surfaceCapabilities);

	width = std::max(surfaceCapabilities.minImageExtent.width, width);
	width = std::min(surfaceCapabilities.maxImageExtent.width, width);
	height = std::max(surfaceCapabilities.minImageExtent.height, height);
	height = std::min(surfaceCapabilities.maxImageExtent.height, height);

	// Find present queue.
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(m_context->getPhysicalDevice(), &queueFamilyCount, 0);

	AutoArrayPtr< VkQueueFamilyProperties > queueFamilyProperties(new VkQueueFamilyProperties[queueFamilyCount]);
	vkGetPhysicalDeviceQueueFamilyProperties(m_context->getPhysicalDevice(), &queueFamilyCount, queueFamilyProperties.ptr());

	uint32_t presentQueueIndex = ~0;
	for (uint32_t i = 0; i < queueFamilyCount; ++i)
	{
		VkBool32 supportsPresent = VK_FALSE;
		vkGetPhysicalDeviceSurfaceSupportKHR(m_context->getPhysicalDevice(), i, m_surface, &supportsPresent);
		if (supportsPresent)
		{
			presentQueueIndex = i;
			break;
		}
	}
	if (presentQueueIndex == ~0)
	{
		log::error << L"Failed to create Vulkan; no suitable present queue found." << Endl;
		return false;
	}

	if (m_context->getGraphicsQueue()->getQueueIndex() != presentQueueIndex)
		m_presentQueue = Queue::create(m_context, presentQueueIndex);
	else
		m_presentQueue = m_context->getGraphicsQueue();

	// Determine primary target color format/space.
	uint32_t surfaceFormatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(m_context->getPhysicalDevice(), m_surface, &surfaceFormatCount, nullptr);
	if (surfaceFormatCount == 0)
	{
		log::error << L"Failed to create Vulkan; no surface formats." << Endl;
		return false;
	}

	AutoArrayPtr< VkSurfaceFormatKHR > surfaceFormats(new VkSurfaceFormatKHR[surfaceFormatCount]);
	vkGetPhysicalDeviceSurfaceFormatsKHR(m_context->getPhysicalDevice(), m_surface, &surfaceFormatCount, surfaceFormats.ptr());

	VkFormat colorFormat = VK_FORMAT_UNDEFINED;
	VkColorSpaceKHR colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	for (uint32_t i = 0; i < surfaceFormatCount; ++i)
	{
		if (surfaceFormats[i].format != VK_FORMAT_B8G8R8A8_SRGB)
		{
			colorFormat = surfaceFormats[i].format;
			colorSpace = surfaceFormats[i].colorSpace;
			break;
		}
	}

	VkExtent2D surfaceResolution =  surfaceCapabilities.currentExtent;
	if (surfaceResolution.width <= -1)
	{
		surfaceResolution.width = width;
		surfaceResolution.height = height;
	}

	VkSurfaceTransformFlagBitsKHR preTransform = surfaceCapabilities.currentTransform;
	if (surfaceCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
		preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;

	// Determine presentation mode and desired number of images.
	VkPresentModeKHR presentationMode = VK_PRESENT_MODE_FIFO_KHR;

#if defined(__IOS__)
	if (presentationModeSupported(m_context->getPhysicalDevice(), m_surface, VK_PRESENT_MODE_MAILBOX_KHR))
	{
		presentationMode = VK_PRESENT_MODE_MAILBOX_KHR;
		desiredImageCount = 3;
	}
#else
	if (vblanks <= 0)
	{
		if (presentationModeSupported(m_context->getPhysicalDevice(), m_surface, VK_PRESENT_MODE_MAILBOX_KHR))
			presentationMode = VK_PRESENT_MODE_MAILBOX_KHR;
		else if (presentationModeSupported(m_context->getPhysicalDevice(), m_surface, VK_PRESENT_MODE_FIFO_RELAXED_KHR))
			presentationMode = VK_PRESENT_MODE_FIFO_RELAXED_KHR;
	}
#endif

	if (vblanks <= 0)
	{
		if (presentationModeSupported(m_context->getPhysicalDevice(), m_surface, VK_PRESENT_MODE_IMMEDIATE_KHR))
			presentationMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
	}

	if (presentationMode == VK_PRESENT_MODE_FIFO_KHR)
		log::debug << L"Using FIFO presentation mode." << Endl;
	else if (presentationMode == VK_PRESENT_MODE_FIFO_RELAXED_KHR)
		log::debug << L"Using FIFO (relaxed) presentation mode." << Endl;
	else if (presentationMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
		log::debug << L"Using IMMEDIATE presentation mode." << Endl;
	else if (presentationMode == VK_PRESENT_MODE_MAILBOX_KHR)
		log::debug << L"Using MAILBOX presentation mode." << Endl;

	// Check so desired image count is supported.
	uint32_t desiredImageCount = 3;
	uint32_t clampedImageCount = desiredImageCount;
	if (clampedImageCount < surfaceCapabilities.minImageCount)
		clampedImageCount = surfaceCapabilities.minImageCount;
	else if (surfaceCapabilities.maxImageCount != 0 && clampedImageCount > surfaceCapabilities.maxImageCount)
		clampedImageCount = surfaceCapabilities.maxImageCount;

	// Create swap chain.
	VkSwapchainCreateInfoKHR scci = {};
	scci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	scci.surface = m_surface;
	scci.minImageCount = clampedImageCount;
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
	scci.oldSwapchain = m_swapChain;

	uint32_t queueFamilyIndices[] = { m_context->getGraphicsQueue()->getQueueIndex(), m_presentQueue->getQueueIndex() };
	if (queueFamilyIndices[0] != queueFamilyIndices[1])
	{
		// Need to be sharing between queues in order to be presentable.
		scci.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		scci.queueFamilyIndexCount = 2;
		scci.pQueueFamilyIndices = queueFamilyIndices;
	}

	if (vkCreateSwapchainKHR(m_context->getLogicalDevice(), &scci, 0, &m_swapChain) != VK_SUCCESS)
	{
		log::error << L"Failed to create Vulkan; unable to create swap chain." << Endl;
		return false;
	}

	// Destroy previous swap chain.
	if (scci.oldSwapchain != 0)
		vkDestroySwapchainKHR(m_context->getLogicalDevice(), scci.oldSwapchain, 0);	

	// Get primary color images.
	uint32_t imageCount = 0;
	vkGetSwapchainImagesKHR(m_context->getLogicalDevice(), m_swapChain, &imageCount, nullptr);

	AlignedVector< VkImage > presentImages(imageCount);
	vkGetSwapchainImagesKHR(m_context->getLogicalDevice(), m_swapChain, &imageCount, presentImages.ptr());

	log::debug << L"Got " << imageCount << L" images in swap chain; requested " << desiredImageCount << L" image(s)." << Endl;

	VkSemaphoreCreateInfo sci = {};
	sci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	if (vkCreateSemaphore(m_context->getLogicalDevice(), &sci, nullptr, &m_imageAvailableSemaphore) != VK_SUCCESS)
		return false;
	setObjectDebugName(m_context->getLogicalDevice(), L"m_imageAvailableSemaphore", (uint64_t)m_imageAvailableSemaphore, VK_OBJECT_TYPE_SEMAPHORE);

#if !defined(__ANDROID__) && !defined(__IOS__)
	// Create time query pool.
	VkQueryPoolCreateInfo qpci = {};
	qpci.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
	qpci.pNext = nullptr;
	qpci.queryType = VK_QUERY_TYPE_TIMESTAMP;
	qpci.queryCount = (imageCount + 1) * 2 * 1024;
	if (vkCreateQueryPool(m_context->getLogicalDevice(), &qpci, nullptr, &m_queryPool) != VK_SUCCESS)
		return false;
#endif

	// Create primary depth target.
	Ref< RenderTargetDepthVk > primaryDepth = new RenderTargetDepthVk(m_context);
	if (!primaryDepth->createPrimary(
		width,
		height,
		multiSample,
#if defined(__IOS__)
		VK_FORMAT_D16_UNORM_S8_UINT,
#elif defined(__RPI__)
		VK_FORMAT_D24_UNORM_S8_UINT,
#else
	    VK_FORMAT_D32_SFLOAT_S8_UINT,
#endif
		L"Primary Depth"
	))
		return false;

	// Create frame resources.
	m_frames.resize(imageCount);
	for (uint32_t i = 0; i < imageCount; ++i)
	{
		auto& frame = m_frames[i];

		VkSemaphoreCreateInfo sci = {};
		sci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		vkCreateSemaphore(m_context->getLogicalDevice(), &sci, nullptr, &frame.renderFinishedSemaphore);
		setObjectDebugName(m_context->getLogicalDevice(), L"frame.renderFinishedSemaphore", (uint64_t)frame.renderFinishedSemaphore, VK_OBJECT_TYPE_SEMAPHORE);

		static uint32_t primaryInstances = 0;
		frame.primaryTarget = new RenderTargetSetVk(m_context, primaryInstances);
		if (!frame.primaryTarget->createPrimary(
			width,
			height,
			multiSample,
			colorFormat,
			presentImages[i],
			primaryDepth,
			str(L"Primary %d", i).c_str()
		))
			return false;
	}

	// Check if debug marker extension is available.
	m_haveDebugMarkers = false;
#if !defined(__ANDROID__) && !defined(__IOS__)
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(m_context->getPhysicalDevice(), nullptr, &extensionCount, nullptr);
	AlignedVector< VkExtensionProperties > extensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(m_context->getPhysicalDevice(), nullptr, &extensionCount, extensions.ptr());
	for (auto extension : extensions)
	{
		if (std::strcmp(extension.extensionName, VK_EXT_DEBUG_MARKER_EXTENSION_NAME) == 0)
		{
			T_DEBUG(L"Found debug marker extension; debug markers enabled.");
			m_haveDebugMarkers = true;
			break;
		}
	}
#endif

	m_renderPassCache = new RenderPassCache(m_context->getLogicalDevice());
	m_nextQueryIndex = 0;
	m_lastQueryIndex = 0;
	m_lost = false;
	return true;
}

bool RenderViewVk::validateGraphicsPipeline(const VertexLayoutVk* vertexLayout, ProgramVk* p, PrimitiveType pt)
{
	auto& frame = m_frames[m_currentImageIndex];
	
	// Calculate pipeline key.
	const uint8_t primitiveId = (uint8_t)pt;
	const uint32_t declHash = vertexLayout->getHash();
	const uint32_t shaderHash = p->getShaderHash();
	const auto key = std::make_tuple(primitiveId, m_targetRenderPassHash, declHash, shaderHash);

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
		const uint32_t colorAttachmentCount = m_targetSet->getColorTargetCount();

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
		visci.pVertexBindingDescriptions = &vertexLayout->getVkVertexInputBindingDescription();
		visci.vertexAttributeDescriptionCount = (uint32_t)vertexLayout->getVkVertexInputAttributeDescriptions().size();
		visci.pVertexAttributeDescriptions = vertexLayout->getVkVertexInputAttributeDescriptions().c_ptr();

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
		rsci.cullMode = c_cullMode[(int32_t)rs.cullMode];
		rsci.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rsci.depthBiasEnable = VK_FALSE;
		rsci.depthBiasConstantFactor = 0;
		rsci.depthBiasClamp = 0;
		rsci.depthBiasSlopeFactor = 0;
		rsci.lineWidth = 1;

		VkPipelineMultisampleStateCreateInfo mssci = {};
		mssci.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		mssci.rasterizationSamples = m_targetSet->getVkSampleCount();
		if (m_multiSampleShading > FUZZY_EPSILON)
		{
			mssci.sampleShadingEnable = VK_TRUE;
			mssci.minSampleShading = m_multiSampleShading;
		}
		else
			mssci.sampleShadingEnable = VK_FALSE;
		mssci.pSampleMask = nullptr;
		mssci.alphaToCoverageEnable = rs.alphaToCoverageEnable ? VK_TRUE : VK_FALSE;
		mssci.alphaToOneEnable = VK_FALSE;

		VkStencilOpState sops = {};
		sops.failOp = c_stencilOperations[(int)rs.stencilFail];
		sops.passOp = c_stencilOperations[(int)rs.stencilPass];
		sops.depthFailOp = c_stencilOperations[(int)rs.stencilZFail];
		sops.compareOp = c_compareOperations[(int)rs.stencilFunction];
		sops.compareMask = rs.stencilMask;
		sops.writeMask = rs.stencilMask;
		sops.reference = rs.stencilReference;

		VkPipelineDepthStencilStateCreateInfo dssci = {};
		dssci.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		dssci.depthTestEnable = rs.depthEnable ? VK_TRUE : VK_FALSE;
		dssci.depthWriteEnable = rs.depthWriteEnable ? VK_TRUE : VK_FALSE;
		dssci.depthCompareOp = rs.depthEnable ? c_compareOperations[(int)rs.depthFunction] : VK_COMPARE_OP_ALWAYS;
		dssci.depthBoundsTestEnable = VK_FALSE;
		dssci.stencilTestEnable = rs.stencilEnable ? VK_TRUE : VK_FALSE;
		dssci.front = sops;
		dssci.back = sops;
		dssci.minDepthBounds = 0;
		dssci.maxDepthBounds = 0;

		StaticVector< VkPipelineColorBlendAttachmentState, RenderTargetSetCreateDesc::MaxTargets > blendAttachments;
		for (uint32_t i = 0; i < colorAttachmentCount; ++i)
		{
			auto& cbas = blendAttachments.push_back();
			cbas.blendEnable = rs.blendEnable ? VK_TRUE : VK_FALSE;
			cbas.srcColorBlendFactor = c_blendFactors[(int)rs.blendColorSource];
			cbas.dstColorBlendFactor = c_blendFactors[(int)rs.blendColorDestination];
			cbas.colorBlendOp = c_blendOperations[(int)rs.blendColorOperation];
			cbas.srcAlphaBlendFactor = c_blendFactors[(int)rs.blendAlphaSource];
			cbas.dstAlphaBlendFactor = c_blendFactors[(int)rs.blendAlphaDestination];
			cbas.alphaBlendOp = c_blendOperations[(int)rs.blendAlphaOperation];
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
		iasci.topology = c_primitiveTopology[(int32_t)pt];
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

		VkResult result = vkCreateGraphicsPipelines(
			m_context->getLogicalDevice(),
			m_context->getPipelineCache(),
			1,
			&gpci,
			nullptr,
			&pipeline
		);
		if (result != VK_SUCCESS)
		{
#if defined(_DEBUG)
			log::error << L"Unable to create Vulkan graphics pipeline (" << getHumanResult(result) << L"), \"" << p->getTag() << L"\"." << Endl;
#else
			log::error << L"Unable to create Vulkan graphics pipeline (" << getHumanResult(result) << L")." << Endl;
#endif
			return false;
		}

		m_pipelines[key] = { m_counter, pipeline };
#if defined(_DEBUG)
		log::debug << L"Graphics pipeline created (" << p->getTag() << L", " << m_pipelines.size() << L" pipelines)." << Endl;
#endif
	}

	if (!pipeline)
		return false;

	if (pipeline != frame.boundPipeline)
	{
		vkCmdBindPipeline(*frame.graphicsCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
		frame.boundPipeline = pipeline;
	}
	return true;
}

bool RenderViewVk::validateComputePipeline(ProgramVk* p)
{
	auto& frame = m_frames[m_currentImageIndex];

	// Calculate pipeline key.
	const uint8_t primitiveId = 0;
	const uint32_t declHash = 0;
	const uint32_t shaderHash = p->getShaderHash();
	const auto key = std::make_tuple(primitiveId, 0, declHash, shaderHash);

	VkPipeline pipeline = 0;

	auto it = m_pipelines.find(key);
	if (it != m_pipelines.end())
	{
		it->second.lastAcquired = m_counter;
		pipeline = it->second.pipeline;
	}
	else
	{
		VkPipelineShaderStageCreateInfo ssci = {};
		ssci.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		ssci.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		ssci.module = p->getComputeVkShaderModule();
		ssci.pName = "main";
		ssci.pSpecializationInfo = nullptr;

		VkComputePipelineCreateInfo cpci = {};
		cpci.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		cpci.stage = ssci;
		cpci.layout = p->getPipelineLayout();

		VkResult result = vkCreateComputePipelines(
			m_context->getLogicalDevice(),
			m_context->getPipelineCache(),
			1,
			&cpci,
			nullptr,
			&pipeline
		);
		if (result != VK_SUCCESS)
		{
#if defined(_DEBUG)
			log::error << L"Unable to create Vulkan compute pipeline (" << getHumanResult(result) << L"), \"" << p->getTag() << L"\"." << Endl;
#else
			log::error << L"Unable to create Vulkan compute pipeline (" << getHumanResult(result) << L")." << Endl;
#endif
			return false;
		}

		m_pipelines[key] = { m_counter, pipeline };
#if defined(_DEBUG)
		log::debug << L"Compute pipeline created (" << p->getTag() << L", " << m_pipelines.size() << L" pipelines)." << Endl;
#endif
	}

	if (!pipeline)
		return false;

	if (pipeline != frame.boundComputePipeline)
	{
		vkCmdBindPipeline(*frame.graphicsCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
		frame.boundComputePipeline = pipeline;
	}
	return true;
}

#if defined(_WIN32)
bool RenderViewVk::windowListenerEvent(Window* window, UINT message, WPARAM wParam, LPARAM lParam, LRESULT& outResult)
{
	if (message == WM_CLOSE)
	{
		RenderEvent evt;
		evt.type = RenderEventType::Close;
		m_eventQueue.push_back(evt);
	}
	else if (message == WM_MOVE)
	{
		// Remove all pending resize events.
		m_eventQueue.remove_if([](const RenderEvent& evt) {
			return evt.type == RenderEventType::Resize;
		});

		RenderEvent evt;
		evt.type = RenderEventType::Resize;
		evt.resize.width = getWidth();
		evt.resize.height = getHeight();
		m_eventQueue.push_back(evt);
	}
	else if (message == WM_SIZE)
	{
		// Remove all pending resize events.
		m_eventQueue.remove_if([](const RenderEvent& evt) {
			return evt.type == RenderEventType::Resize;
		});

		// Push new resize event if not matching current size.
		int32_t width = LOWORD(lParam);
		int32_t height = HIWORD(lParam);

		if (width <= 0 || height <= 0)
			return false;

		if (width != getWidth() || height != getHeight())
		{
			RenderEvent evt;
			evt.type = RenderEventType::Resize;
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
			evt.type = RenderEventType::ToggleFullScreen;
			m_eventQueue.push_back(evt);
		}
	}
	else if (message == WM_KEYDOWN)
	{
		if (wParam == VK_RETURN && (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0)
		{
			RenderEvent evt;
			evt.type = RenderEventType::ToggleFullScreen;
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
