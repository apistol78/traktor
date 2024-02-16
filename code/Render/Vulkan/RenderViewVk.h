/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <list>
#include <tuple>
#include "Core/System.h"
#include "Core/Containers/AlignedVector.h"
#include "Render/IRenderView.h"
#include "Render/Vulkan/Private/ApiHeader.h"
#if defined(_WIN32)
#	include "Render/Vulkan/Win32/Window.h"
#elif defined(__LINUX__) || defined(__RPI__)
#	include "Render/Vulkan/Linux/Window.h"
#elif defined(__MAC__)
#	include "Render/Vulkan/macOS/Window.h"
#endif

namespace traktor::render
{

class BufferViewVk;
class CommandBuffer;
class Context;
class ProgramVk;
class Queue;
class RenderPassCache;
class RenderTargetSetVk;
class VertexLayoutVk;

/*!
 * \ingroup Vulkan
 */
class RenderViewVk
:	public IRenderView
#if defined(_WIN32)
,	public IWindowListener
#endif
{
	T_RTTI_CLASS;

public:
	explicit RenderViewVk(
		Context* context,
		VkInstance instance
	);

	virtual ~RenderViewVk();

	bool create(const RenderViewDefaultDesc& desc);

	bool create(const RenderViewEmbeddedDesc& desc);

	virtual bool nextEvent(RenderEvent& outEvent) override final;

	virtual void close() override final;

	virtual bool reset(const RenderViewDefaultDesc& desc) override final;

	virtual bool reset(int32_t width, int32_t height) override final;

	virtual uint32_t getDisplay() const override final;

	virtual int getWidth() const override final;

	virtual int getHeight() const override final;

	virtual bool isActive() const override final;

	virtual bool isMinimized() const override final;

	virtual bool isFullScreen() const override final;

	virtual void showCursor() override final;

	virtual void hideCursor() override final;

	virtual bool isCursorVisible() const override final;

	virtual bool setGamma(float gamma) override final;

	virtual void setViewport(const Viewport& viewport) override final;

	virtual SystemWindow getSystemWindow() override final;

	virtual bool beginFrame() override final;

	virtual void endFrame() override final;

	virtual void present() override final;

	virtual bool beginPass(const Clear* clear, uint32_t load, uint32_t store) override final;

	virtual bool beginPass(IRenderTargetSet* renderTargetSet, const Clear* clear, uint32_t load, uint32_t store) override final;

	virtual bool beginPass(IRenderTargetSet* renderTargetSet, int32_t renderTarget, const Clear* clear, uint32_t load, uint32_t store) override final;

	virtual void endPass() override final;

	virtual void draw(const IBufferView* vertexBuffer, const IVertexLayout* vertexLayout, const IBufferView* indexBuffer, IndexType indexType, IProgram* program, const Primitives& primitives, uint32_t instanceCount) override final;

	virtual void drawIndirect(const IBufferView* vertexBuffer, const IVertexLayout* vertexLayout, const IBufferView* indexBuffer, IndexType indexType, IProgram* program, PrimitiveType primitiveType, const IBufferView* drawBuffer, uint32_t drawCount) override final;

	virtual void compute(IProgram* program, const int32_t* workSize) override final;

	virtual void barrier(Stage from, Stage to) override final;

	virtual bool copy(ITexture* destinationTexture, const Region& destinationRegion, ITexture* sourceTexture, const Region& sourceRegion) override final;

	virtual int32_t beginTimeQuery() override final;

	virtual void endTimeQuery(int32_t query) override final;

	virtual bool getTimeQuery(int32_t query, bool wait, double& outStart, double& outEnd) const override final;

	virtual void pushMarker(const std::wstring& marker) override final;

	virtual void popMarker() override final;

	virtual void getStatistics(RenderViewStatistics& outStatistics) const override final;

private:
	typedef std::tuple< uint8_t, uint32_t, uint32_t, uint32_t > pipeline_key_t;

	struct Frame
	{
		Ref< CommandBuffer > graphicsCommandBuffer;
		VkSemaphore renderFinishedSemaphore;
		Ref< RenderTargetSetVk > primaryTarget;

		VkPipeline boundPipeline = 0;
		VkPipeline boundComputePipeline = 0;
		BufferViewVk boundIndexBuffer;
		BufferViewVk boundVertexBuffer;

		std::list< std::string > markers;
		AlignedVector< bool > markerStack;
	};

	struct PipelineEntry
	{
		uint32_t lastAcquired;
		VkPipeline pipeline;
	};

	Context* m_context = nullptr;
	VkInstance m_instance = 0;
#if defined(_WIN32) || defined(__LINUX__) || defined(__RPI__) || defined(__MAC__)
	Ref< Window > m_window;
#endif
	VkSurfaceKHR m_surface = 0;
	Ref< Queue > m_presentQueue;
#if !defined(__ANDROID__) && !defined(__IOS__)
	VkQueryPool m_queryPool = 0;
#endif
	bool m_lost = true;

	// Swap chain.
	VkSwapchainKHR m_swapChain = 0;
	VkSemaphore m_imageAvailableSemaphore = 0;
	AlignedVector< Frame > m_frames;
	uint32_t m_currentImageIndex = 0;
	uint32_t m_multiSample = 0;
	float m_multiSampleShading = 0.0f;
	int32_t m_vblanks = 0;

	// Event queue.
	std::list< RenderEvent > m_eventQueue;

	// Render pass cache.
	Ref< RenderPassCache > m_renderPassCache;
	
	// Current pass's target.
	Ref< RenderTargetSetVk > m_targetSet;
	int32_t m_targetColorIndex = 0;
	VkRenderPass m_targetRenderPass = 0;
	VkFramebuffer m_targetFrameBuffer = 0;
	uint32_t m_targetRenderPassHash = 0;

	// Pipelines.
	SmallMap< pipeline_key_t, PipelineEntry > m_pipelines;

	// Stats.
	bool m_haveDebugMarkers = false;
	bool m_cursorVisible = true;
	int32_t m_nextQueryIndex = 0;
	int32_t m_lastQueryIndex = 0;
	uint32_t m_counter = -1;
	uint32_t m_passCount = 0;
	uint32_t m_drawCalls = 0;
	uint32_t m_primitiveCount = 0;

	bool create(uint32_t width, uint32_t height, uint32_t multiSample, float multiSampleShading, int32_t vblanks);

	bool validateGraphicsPipeline(const VertexLayoutVk* vertexLayout, ProgramVk* p, PrimitiveType pt);

	bool validateComputePipeline(ProgramVk* p);

#if defined(_WIN32)
	// \name IWindowListener implementation.
	// \{

	virtual bool windowListenerEvent(Window* window, UINT message, WPARAM wParam, LPARAM lParam, LRESULT& outResult) override final;

	// \}
#endif
};

}
