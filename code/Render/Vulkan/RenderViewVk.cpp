#include "Core/Misc/SafeDestroy.h"
#include "Render/Vulkan/RenderTargetDepthVk.h"
#include "Render/Vulkan/RenderTargetVk.h"
#include "Render/Vulkan/RenderTargetSetVk.h"
#include "Render/Vulkan/RenderViewVk.h"
#if defined(_WIN32)
#	include "Render/Vulkan/Win32/ApiLoader.h"
#	include "Render/Vulkan/Win32/Window.h"
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

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderViewVk", RenderViewVk, IRenderView)

#if defined(_WIN32)
RenderViewVk::RenderViewVk(
	Window* window,
	VkDevice device,
	VkSwapchainKHR swapChain,
	VkQueue presentQueue,
	VkCommandBuffer drawCmdBuffer,
	const AlignedVector< VkImage >& presentImages,
	VkImage depthImage,
	VkRenderPass renderPass,
	const AlignedVector< VkFramebuffer >& frameBuffers
)
:	m_window(window)
,	m_device(device)
,	m_swapChain(swapChain)
,	m_presentQueue(presentQueue)
,	m_currentImageIndex(0)
,	m_drawCmdBuffer(drawCmdBuffer)
,	m_presentImages(presentImages)
,	m_depthImage(depthImage)
,	m_renderPass(renderPass)
,	m_frameBuffers(frameBuffers)
,	m_presentCompleteSemaphore(0)
,	m_renderingCompleteSemaphore(0)
{
	if (m_window)
		m_window->addListener(this);
}
#else
RenderViewVk::RenderViewVk(VkDevice device)
:	m_device(device)
{
}
#endif

RenderViewVk::~RenderViewVk()
{
	close();
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
#endif

	if (!m_eventQueue.empty())
	{
		outEvent = m_eventQueue.front();
		m_eventQueue.pop_front();
		return true;
	}
	else
		return false;
}

void RenderViewVk::close()
{
}

bool RenderViewVk::reset(const RenderViewDefaultDesc& desc)
{
#if defined(_WIN32)

	// Cannot reset embedded view.
	if (!m_window)
		return false;

	m_window->removeListener(this);
	m_window->setTitle(!desc.title.empty() ? desc.title.c_str() : L"Traktor - Vulkan Renderer");
	m_window->addListener(this);

#endif
	return true;
}

bool RenderViewVk::reset(int32_t width, int32_t height)
{
	return false;
}

int RenderViewVk::getWidth() const
{
	return 0;
}

int RenderViewVk::getHeight() const
{
	return 0;
}

bool RenderViewVk::isActive() const
{
	return true;
}

bool RenderViewVk::isMinimized() const
{
	return false;
}

bool RenderViewVk::isFullScreen() const
{
	return true;
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
}

Viewport RenderViewVk::getViewport()
{
	return Viewport();
}

SystemWindow RenderViewVk::getSystemWindow()
{
	SystemWindow sw;
	return sw;
}

bool RenderViewVk::begin(EyeType eye)
{
#if defined(_WIN32)
    VkSemaphoreCreateInfo semaphoreCreateInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, 0, 0 };
    vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr, &m_presentCompleteSemaphore);
    vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr, &m_renderingCompleteSemaphore);

	// Get next target from swap chain.
    vkAcquireNextImageKHR(m_device, m_swapChain, UINT64_MAX, m_presentCompleteSemaphore, VK_NULL_HANDLE, &m_currentImageIndex);

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(m_drawCmdBuffer, &beginInfo);
	
	// change image layout from VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
	// to VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	VkImageMemoryBarrier layoutTransitionBarrier = {};
	layoutTransitionBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	layoutTransitionBarrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	layoutTransitionBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	layoutTransitionBarrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	layoutTransitionBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	layoutTransitionBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	layoutTransitionBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	layoutTransitionBarrier.image = m_presentImages[m_currentImageIndex];
	VkImageSubresourceRange resourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	layoutTransitionBarrier.subresourceRange = resourceRange;
	
	vkCmdPipelineBarrier(
		m_drawCmdBuffer, 
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 
		0,
		0, nullptr,
		0, nullptr, 
		1, &layoutTransitionBarrier
	);

	VkClearValue clearValue[] = { { 1.0f, 0.0f, 0.0f, 1.0f }, { 1.0, 0.0 } };

	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = m_renderPass;
	renderPassBeginInfo.framebuffer = m_frameBuffers[m_currentImageIndex];
	renderPassBeginInfo.renderArea = { 0, 0, 1920, 1080 };
	renderPassBeginInfo.clearValueCount = 2;
	renderPassBeginInfo.pClearValues = clearValue;
	vkCmdBeginRenderPass(m_drawCmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
#endif
	return true;
}

bool RenderViewVk::begin(RenderTargetSet* renderTargetSet)
{
	return true;
}

bool RenderViewVk::begin(RenderTargetSet* renderTargetSet, int renderTarget)
{
	return true;
}

void RenderViewVk::clear(uint32_t clearMask, const Color4f* colors, float depth, int32_t stencil)
{
}

void RenderViewVk::draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives)
{
}

void RenderViewVk::draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives, uint32_t instanceCount)
{
}

void RenderViewVk::end()
{
}

void RenderViewVk::present()
{
#if defined(_WIN32)
	VkImageMemoryBarrier prePresentBarrier = {};
	prePresentBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	prePresentBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	prePresentBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	prePresentBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	prePresentBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	prePresentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	prePresentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	prePresentBarrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
	prePresentBarrier.image = m_presentImages[m_currentImageIndex];
    
	vkCmdPipelineBarrier(
		m_drawCmdBuffer, 
		VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 
		0, 
		0, nullptr, 
		0, nullptr, 
		1, &prePresentBarrier
	);
 
	vkEndCommandBuffer(m_drawCmdBuffer);

    VkFence renderFence;
    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    vkCreateFence(m_device, &fenceCreateInfo, nullptr, &renderFence);
 
    VkPipelineStageFlags waitStageMash = { VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT };
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &m_presentCompleteSemaphore;
    submitInfo.pWaitDstStageMask = &waitStageMash;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_drawCmdBuffer;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &m_renderingCompleteSemaphore;
    vkQueueSubmit(m_presentQueue, 1, &submitInfo, renderFence);
 
    vkWaitForFences(m_device, 1, &renderFence, VK_TRUE, UINT64_MAX);
    vkDestroyFence(m_device, renderFence, nullptr);
 
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &m_renderingCompleteSemaphore;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &m_swapChain;
    presentInfo.pImageIndices = &m_currentImageIndex;
    presentInfo.pResults = nullptr;
    vkQueuePresentKHR(m_presentQueue, &presentInfo );
 
    vkDestroySemaphore(m_device, m_presentCompleteSemaphore, nullptr);
    vkDestroySemaphore(m_device, m_renderingCompleteSemaphore, nullptr);
#endif
}

void RenderViewVk::pushMarker(const char* const marker)
{
}

void RenderViewVk::popMarker()
{
}

void RenderViewVk::getStatistics(RenderViewStatistics& outStatistics) const
{
	outStatistics.drawCalls = 0;
	outStatistics.primitiveCount = 0;
}

bool RenderViewVk::getBackBufferContent(void* buffer) const
{
	return false;
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
