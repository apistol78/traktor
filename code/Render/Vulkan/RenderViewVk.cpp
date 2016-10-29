#include "Core/Misc/SafeDestroy.h"
#include "Render/Vulkan/IndexBufferVk.h"
#include "Render/Vulkan/ProgramVk.h"
#include "Render/Vulkan/RenderTargetDepthVk.h"
#include "Render/Vulkan/RenderTargetVk.h"
#include "Render/Vulkan/RenderTargetSetVk.h"
#include "Render/Vulkan/RenderViewVk.h"
#include "Render/Vulkan/VertexBufferVk.h"
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
	VkDescriptorSetLayout descriptorSetLayout,
	VkPipelineLayout pipelineLayout,
	VkDescriptorPool descriptorPool,
	VkDescriptorSet descriptorSet,
	const RefArray< RenderTargetSetVk >& primaryTargets
)
:	m_window(window)
,	m_device(device)
,	m_swapChain(swapChain)
,	m_presentQueue(presentQueue)
,	m_currentImageIndex(0)
,	m_drawCmdBuffer(drawCmdBuffer)
,	m_descriptorSetLayout(descriptorSetLayout)
,	m_pipelineLayout(pipelineLayout)
,	m_descriptorPool(descriptorPool)
,	m_descriptorSet(descriptorSet)
,	m_primaryTargets(primaryTargets)
,	m_presentCompleteSemaphore(0)
,	m_renderingCompleteSemaphore(0)
,	m_targetStateDirty(false)
,	m_pipeline(0)
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
	return m_primaryTargets.front()->getWidth();
}

int RenderViewVk::getHeight() const
{
	return m_primaryTargets.front()->getHeight();
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
	return SystemWindow(*m_window);
}

bool RenderViewVk::begin(EyeType eye)
{
    VkSemaphoreCreateInfo semaphoreCreateInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, 0, 0 };
    vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr, &m_presentCompleteSemaphore);
    vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr, &m_renderingCompleteSemaphore);

	// Get next target from swap chain.
    vkAcquireNextImageKHR(m_device, m_swapChain, UINT64_MAX, m_presentCompleteSemaphore, VK_NULL_HANDLE, &m_currentImageIndex);

	// Begin recording command buffer.
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(m_drawCmdBuffer, &beginInfo);
	
	// Push primary target onto stack.
	TargetState ts;
	ts.rts = m_primaryTargets[m_currentImageIndex];
	ts.colorIndex = 0;
	ts.clearMask = 0;

	m_targetStateStack.push_back(ts);
	m_targetStateDirty = true;

	return true;
}

bool RenderViewVk::begin(RenderTargetSet* renderTargetSet)
{
	TargetState ts;
	ts.rts = mandatory_non_null_type_cast< RenderTargetSetVk* >(renderTargetSet);
	ts.colorIndex = 0;
	ts.clearMask = 0;

	m_targetStateStack.push_back(ts);
	m_targetStateDirty = true;

	return true;
}

bool RenderViewVk::begin(RenderTargetSet* renderTargetSet, int renderTarget)
{
	TargetState ts;
	ts.rts = mandatory_non_null_type_cast< RenderTargetSetVk* >(renderTargetSet);
	ts.colorIndex = renderTarget;
	ts.clearMask = 0;

	m_targetStateStack.push_back(ts);
	m_targetStateDirty = true;

	return true;
}

void RenderViewVk::clear(uint32_t clearMask, const Color4f* colors, float depth, int32_t stencil)
{
	TargetState& ts = m_targetStateStack.back();
	if (m_targetStateDirty)
	{
		ts.clearMask |= clearMask;
	}
	else
	{
		// Target state already validated; clear again or begin/end render pass?
	}
}

void RenderViewVk::draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives)
{
	VertexBufferVk* vb = mandatory_non_null_type_cast< VertexBufferVk* >(vertexBuffer);
	ProgramVk* p = mandatory_non_null_type_cast< ProgramVk* >(program);

	// Validate render pass, framebuffer and graphics pipeline.
	validateTargetState();
	validatePipeline(vb, p, primitives.type);

	p->validate(m_device, m_descriptorSet);

	vkCmdBindDescriptorSets(
		m_drawCmdBuffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		m_pipelineLayout,
		0,
		1, &m_descriptorSet,
		0, nullptr
	);

	uint32_t vertexCount = 0;
	switch (primitives.type)
	{
	case PtPoints:
		vertexCount = primitives.count;
		break;

	case PtLineStrip:
		T_ASSERT (0);
		break;

	case PtLines:
		vertexCount = primitives.count * 2;
		break;

	case PtTriangleStrip:
		vertexCount = primitives.count + 2;
		break;

	case PtTriangles:
		vertexCount = primitives.count * 3;
		break;
	}

	VkBuffer vbb = vb->getVkBuffer();
    VkDeviceSize offsets = {};
    vkCmdBindVertexBuffers(m_drawCmdBuffer, 0, 1, &vbb, &offsets);
 
	if (indexBuffer && primitives.indexed)
	{
		IndexBufferVk* ib = mandatory_non_null_type_cast<IndexBufferVk*>(indexBuffer);
		VkBuffer ibb = ib->getVkBuffer();
		VkDeviceSize offset = {};
		vkCmdBindIndexBuffer(m_drawCmdBuffer, ibb, offset, (ib->getIndexType() == ItUInt16) ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32);

		vkCmdDrawIndexed(
			m_drawCmdBuffer,
			vertexCount,	// index count
			1,	// instance count
			primitives.offset,	// first index
			0,	// vertex offset
			0	// first instance id
		);
	}
	else
	{
		vkCmdDraw(
			m_drawCmdBuffer,
			vertexCount,   // vertex count
			1,   // instance count
			primitives.offset,   // first vertex
			0 // first instance
		);
	}
}

void RenderViewVk::draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives, uint32_t instanceCount)
{
	validateTargetState();
}

void RenderViewVk::end()
{
	// Close current render pass if it has begun.
	if (!m_targetStateDirty)
		vkCmdEndRenderPass(m_drawCmdBuffer);

	// Pop previous render pass from stack.
	m_targetStateStack.pop_back();
	m_targetStateDirty = true;
}

void RenderViewVk::present()
{
	T_FATAL_ASSERT (m_targetStateStack.empty());

	// Prepare primary color for presentation.
	m_primaryTargets[m_currentImageIndex]->getColorTargetVk(0)->prepareForPresentation(m_drawCmdBuffer);
 
	// End recording command buffer.
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
    vkQueuePresentKHR(m_presentQueue, &presentInfo);
 
    vkDestroySemaphore(m_device, m_presentCompleteSemaphore, nullptr);
    vkDestroySemaphore(m_device, m_renderingCompleteSemaphore, nullptr);
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

void RenderViewVk::validateTargetState()
{
	if (!m_targetStateDirty)
		return;

	T_FATAL_ASSERT (!m_targetStateStack.empty());
	TargetState& ts = m_targetStateStack.back();

	// Prepare primary color for target rendering.
	ts.rts->getColorTargetVk(ts.colorIndex)->prepareAsTarget(m_drawCmdBuffer);

	// Bind render pass and framebuffer.
	VkClearValue clearValue[] = { { 1.0f, 0.0f, 0.0f, 1.0f }, { 1.0, 0.0 } };
	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = ts.rts->getVkRenderPass();
	renderPassBeginInfo.framebuffer = ts.rts->getVkFramebuffer();
	renderPassBeginInfo.renderArea.offset.x = 0;
	renderPassBeginInfo.renderArea.offset.y = 0;
	renderPassBeginInfo.renderArea.extent.width = ts.rts->getWidth();
	renderPassBeginInfo.renderArea.extent.height = ts.rts->getHeight();
	renderPassBeginInfo.clearValueCount = 2;
	renderPassBeginInfo.pClearValues = clearValue;
	vkCmdBeginRenderPass(m_drawCmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	m_targetStateDirty = false;
}

bool RenderViewVk::validatePipeline(VertexBufferVk* vb, ProgramVk* p, PrimitiveType pt)
{
	T_FATAL_ASSERT (!m_targetStateStack.empty());
	TargetState& ts = m_targetStateStack.back();

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = ts.rts->getWidth();
	viewport.height = ts.rts->getHeight();
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
 
	VkRect2D scissors = {};
	scissors.offset.x = 0;
	scissors.offset.y = 0;
	scissors.extent.width = ts.rts->getWidth();
	scissors.extent.height = ts.rts->getHeight();
 
	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissors;

	VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = {};
	vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
	vertexInputStateCreateInfo.pVertexBindingDescriptions = &vb->getVkVertexInputBindingDescription();
	vertexInputStateCreateInfo.vertexAttributeDescriptionCount = vb->getVkVertexInputAttributeDescriptions().size();
	vertexInputStateCreateInfo.pVertexAttributeDescriptions = vb->getVkVertexInputAttributeDescriptions().c_ptr();

	VkPipelineShaderStageCreateInfo shaderStageCreateInfo[2] = {};
	shaderStageCreateInfo[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageCreateInfo[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shaderStageCreateInfo[0].module = p->getVertexVkShaderModule();
	shaderStageCreateInfo[0].pName = "main";
	shaderStageCreateInfo[0].pSpecializationInfo = nullptr;
	shaderStageCreateInfo[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageCreateInfo[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStageCreateInfo[1].module = p->getFragmentVkShaderModule();
	shaderStageCreateInfo[1].pName = "main";
	shaderStageCreateInfo[1].pSpecializationInfo = nullptr;

	VkPipelineRasterizationStateCreateInfo rasterizationState = {};
	rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationState.depthClampEnable = VK_FALSE;
	rasterizationState.rasterizerDiscardEnable = VK_FALSE;
	rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizationState.cullMode = VK_CULL_MODE_NONE;
	rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizationState.depthBiasEnable = VK_FALSE;
	rasterizationState.depthBiasConstantFactor = 0;
	rasterizationState.depthBiasClamp = 0;
	rasterizationState.depthBiasSlopeFactor = 0;
	rasterizationState.lineWidth = 1;

	VkPipelineMultisampleStateCreateInfo multisampleState = {};
	multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampleState.sampleShadingEnable = VK_FALSE;
	multisampleState.minSampleShading = 0;
	multisampleState.pSampleMask = NULL;
	multisampleState.alphaToCoverageEnable = VK_FALSE;
	multisampleState.alphaToOneEnable = VK_FALSE;	

	VkStencilOpState noOPStencilState = {};
	noOPStencilState.failOp = VK_STENCIL_OP_KEEP;
	noOPStencilState.passOp = VK_STENCIL_OP_KEEP;
	noOPStencilState.depthFailOp = VK_STENCIL_OP_KEEP;
	noOPStencilState.compareOp = VK_COMPARE_OP_ALWAYS;
	noOPStencilState.compareMask = 0;
	noOPStencilState.writeMask = 0;
	noOPStencilState.reference = 0;
	 
	VkPipelineDepthStencilStateCreateInfo depthState = {};
	depthState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthState.depthTestEnable = VK_TRUE;
	depthState.depthWriteEnable = VK_TRUE;
	depthState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	depthState.depthBoundsTestEnable = VK_FALSE;
	depthState.stencilTestEnable = VK_FALSE;
	depthState.front = noOPStencilState;
	depthState.back = noOPStencilState;
	depthState.minDepthBounds = 0;
	depthState.maxDepthBounds = 0;

	VkPipelineColorBlendAttachmentState colorBlendAttachmentState = {};
	colorBlendAttachmentState.blendEnable = VK_FALSE;
	colorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_COLOR;
	colorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
	colorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachmentState.colorWriteMask = 0xf;
	 
	VkPipelineColorBlendStateCreateInfo colorBlendState = {};
	colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendState.logicOpEnable = VK_FALSE;
	colorBlendState.logicOp = VK_LOGIC_OP_CLEAR;
	colorBlendState.attachmentCount = 1;
	colorBlendState.pAttachments = &colorBlendAttachmentState;
	colorBlendState.blendConstants[0] = 0.0;
	colorBlendState.blendConstants[1] = 0.0;
	colorBlendState.blendConstants[2] = 0.0;
	colorBlendState.blendConstants[3] = 0.0;

/*	VkDynamicState dynamicState[2] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {};
	dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicStateCreateInfo.dynamicStateCount = 2;
	dynamicStateCreateInfo.pDynamicStates = dynamicState;*/	

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo = {};
	inputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

	VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.stageCount = 2;
	pipelineCreateInfo.pStages = shaderStageCreateInfo;
	pipelineCreateInfo.pVertexInputState = &vertexInputStateCreateInfo;
	pipelineCreateInfo.pInputAssemblyState = &inputAssemblyStateCreateInfo;
	pipelineCreateInfo.pTessellationState = nullptr;
	pipelineCreateInfo.pViewportState = &viewportState;
	pipelineCreateInfo.pRasterizationState = &rasterizationState;
	pipelineCreateInfo.pMultisampleState = &multisampleState;
	pipelineCreateInfo.pDepthStencilState = &depthState;
	pipelineCreateInfo.pColorBlendState = &colorBlendState;
	pipelineCreateInfo.pDynamicState = nullptr; // &dynamicStateCreateInfo;
	pipelineCreateInfo.layout = m_pipelineLayout;
	pipelineCreateInfo.renderPass = ts.rts->getVkRenderPass();
	pipelineCreateInfo.subpass = 0;
	pipelineCreateInfo.basePipelineHandle = nullptr;
	pipelineCreateInfo.basePipelineIndex = 0;
	 
	if (vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &m_pipeline) != VK_SUCCESS)
		return false;

	vkCmdBindPipeline(m_drawCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
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
