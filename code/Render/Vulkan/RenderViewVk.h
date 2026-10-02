/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/RefArray.h"
#include "Core/System.h"
#include "Core/Thread/Semaphore.h"
#include "Render/IRenderView.h"
#include "Render/Vulkan/BufferViewVk.h"
#include "Render/Vulkan/Private/ApiHeader.h"

#include <list>
#include <tuple>
#if defined(_WIN32)
#	include "Render/Vulkan/Win32/Window.h"
#elif defined(__LINUX__) || defined(__RPI__)
#	include "Render/Vulkan/Linux/Window.h"
#elif defined(__MAC__)
#	include "Render/Vulkan/macOS/Window.h"
#endif

namespace traktor::render
{

class CommandBuffer;
class Context;
class ProgramVk;
class Queue;
class RenderTargetSetVk;
class VertexLayoutVk;

/*!
 * \ingroup Render
 */
class RenderViewVk
	: public IRenderView
#if defined(_WIN32)
	, public Window::IListener
#endif
{
	T_RTTI_CLASS;

public:
	explicit RenderViewVk(Context* context);

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

	virtual bool isHDR() const override final;

	virtual void setViewport(const Viewport& viewport) override final;

	virtual void setScissor(const Rectangle& scissor) override final;

	virtual SystemWindow getSystemWindow() override final;

	virtual bool beginFrame() override final;

	virtual void endFrame() override final;

	virtual void present() override final;

	virtual bool beginPass(const Clear* clear, uint32_t load, uint32_t store) override final;

	virtual bool beginPass(IRenderTargetSet* renderTargetSet, const Clear* clear, uint32_t load, uint32_t store) override final;

	virtual bool beginPass(IRenderTargetSet* renderTargetSet, int32_t renderTarget, const Clear* clear, uint32_t load, uint32_t store) override final;

	virtual void endPass() override final;

	virtual void clear(const Clear* clear, const Rectangle& rectangle) override final;

	virtual void draw(const IBufferView* vertexBuffer, const IVertexLayout* vertexLayout, const IBufferView* indexBuffer, IndexType indexType, IProgram* program, const Primitives& primitives, uint32_t instanceCount) override final;

	virtual void drawIndirect(const IBufferView* vertexBuffer, const IVertexLayout* vertexLayout, const IBufferView* indexBuffer, IndexType indexType, IProgram* program, PrimitiveType primitiveType, const IBufferView* drawBuffer, uint32_t drawOffset, uint32_t drawCount) override final;

	virtual void compute(IProgram* program, const int32_t* workSize, bool asynchronous) override final;

	virtual void computeIndirect(IProgram* program, const IBufferView* workBuffer, uint32_t workOffset) override final;

	virtual void barrier(Stage from, Stage to, ITexture* written, uint32_t writtenMip, bool asynchronous) override final;

	virtual void synchronize() override final;

	virtual ComputeHandle signalAsynchronousCompute() override final;

	virtual void waitAsynchronousCompute(ComputeHandle handle) override final;

	virtual bool copy(ITexture* destinationTexture, const Region& destinationRegion, ITexture* sourceTexture, const Region& sourceRegion) override final;

	virtual void writeAccelerationStructure(IAccelerationStructure* accelerationStructure, const AlignedVector< IAccelerationStructure::Instance >& instances, bool asynchronous) override final;

	virtual void writeAccelerationStructure(IAccelerationStructure* accelerationStructure, const IBufferView* vertexBuffer, const IVertexLayout* vertexLayout, const IBufferView* indexBuffer, IndexType indexType, const AlignedVector< RaytracingPrimitives >& primitives, bool rebuild, bool asynchronous) override final;

	virtual int32_t beginTimeQuery(bool asynchronous) override final;

	virtual void endTimeQuery(int32_t query, bool asynchronous) override final;

	virtual bool getTimeQuery(int32_t query, bool wait, double& outStart, double& outEnd) const override final;

	virtual void pushMarker(bool asynchronous, const std::wstring& marker) override final;

	virtual void popMarker(bool asynchronous) override final;

	virtual void writeMarker(bool asynchronous, const std::wstring& marker) override final;

	virtual void getStatistics(RenderViewStatistics& outStatistics) const override final;

	Context* getContext() const { return m_context; }

	CommandBuffer* getGraphicsCommandBuffer();

	/*! Forget cached bindings; must be called after recording directly into the view's command buffers. */
	void invalidateBindings();

private:
	//! Time query stamp as resolved with VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WITH_AVAILABILITY_BIT.
	struct TimeQueryStamp
	{
		uint64_t value;
		uint64_t available;
	};

	struct Frame
	{
		Ref< CommandBuffer > graphicsCommandBuffer;
		Ref< CommandBuffer > computeCommandBuffer;
		VkSemaphore renderFinishedSemaphore;
		VkSemaphore computeFinishedSemaphore;
		Ref< RenderTargetSetVk > primaryTarget;
		VkPipeline boundGraphicsPipeline = 0;
		VkPipeline boundComputePipeline = 0;	//!< Compute pipeline bound in boundComputeCommandBuffer.
		CommandBuffer* boundComputeCommandBuffer = nullptr;
		VkPipeline boundAsyncComputePipeline = 0;	//!< Compute pipeline bound in boundAsyncComputeCommandBuffer.
		CommandBuffer* boundAsyncComputeCommandBuffer = nullptr;
		BufferViewVk boundIndexBuffer;
		BufferViewVk boundVertexBuffer;
		RefArray< CommandBuffer > flyingGraphicsCommandBuffers;	//!< Graphics command buffers submitted by splits during the frame.
		RefArray< CommandBuffer > flyingComputeCommandBuffers;	//!< Compute command buffers submitted by splits during the frame.
		RefArray< CommandBuffer > spareGraphicsCommandBuffers;	//!< Consumed graphics command buffers, reused by the next splits.
		RefArray< CommandBuffer > spareComputeCommandBuffers;	//!< Consumed compute command buffers, reused by the next splits.
		std::list< std::string > markers;
		AlignedVector< bool > markerStack;
		uint64_t computeRecordValue = 0;	//!< Timeline value of the open (not yet submitted) asynchronous compute batch; 0 if none open.
		uint64_t computeSubmittedValue = 0;	//!< Highest asynchronous compute batch value already submitted to the compute queue this frame.
		uint64_t graphicsWaitedValue = 0;	//!< Highest asynchronous compute batch value graphics work recorded from now on already waits upon this frame.
		int32_t queryCount = 0;							//!< Number of time query stamps written into the frame's query segment.
		AlignedVector< TimeQueryStamp > queryStamps;	//!< Stamps written by the previous frame rendered with the frame's query segment.
	};

	//! Most recently validated graphics pipeline, and its key.
	struct LastPipeline
	{
		VkPipeline pipeline = 0;
		VkRenderPass renderPass = 0;
		uint32_t declHash = 0;
		uint32_t shaderHash = 0;
		PrimitiveType primitiveType = PrimitiveType::Points;
	};

	Context* m_context = nullptr;
#if defined(_WIN32) || defined(__LINUX__) || defined(__RPI__) || defined(__MAC__)
	Ref< Window > m_window;
#endif
	VkSurfaceKHR m_surface = 0;
	Ref< Queue > m_presentQueue;
#if !defined(__ANDROID__) && !defined(__IOS__)
	VkQueryPool m_queryPool = 0;
	bool m_asynchronousTimeQueries = false;	//!< Time queries can be recorded on the asynchronous compute queue.
#endif
	bool m_lost = true;

	VkPhysicalDeviceProperties m_deviceProperties;

	// Swap chain.
	VkSwapchainKHR m_swapChain = 0;

	// Binary acquire semaphores; one per swap chain image plus a spare passed to the next acquire,
	// after which the spare and the acquired image's slot are swapped.
	AlignedVector< VkSemaphore > m_imageAvailableSemaphores;
	VkSemaphore m_imageAvailableSemaphoreFree = 0;
	AlignedVector< VkSemaphore > m_retiredImageAvailableSemaphores;
	AlignedVector< Frame > m_frames;
	uint32_t m_currentImageIndex = 0;
	uint32_t m_multiSample = 0;
	float m_multiSampleShading = 0.0f;
	int32_t m_vblanks = 0;
	bool m_allowHDR = false;
	bool m_hdr = false;

	// Surface state, queried once per surface and kept across resets.
	bool m_surfaceCacheValid = false;
	VkFormat m_colorFormat = VK_FORMAT_UNDEFINED;
	VkColorSpaceKHR m_colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	VkPresentModeKHR m_presentModeVSync = VK_PRESENT_MODE_FIFO_KHR;
	VkPresentModeKHR m_presentModeNoVSync = VK_PRESENT_MODE_FIFO_KHR;
	uint32_t m_presentQueueFamilyIndex = ~0u;

	// System window event queue.
	Semaphore m_eventQueueLock;
	std::list< RenderEvent > m_eventQueue;

	// Current pass's target.
	Ref< RenderTargetSetVk > m_targetSet;
	int32_t m_targetColorIndex = 0;
	VkRenderPass m_targetRenderPass = 0;
	VkFramebuffer m_targetFrameBuffer = 0;

	// Pipeline lookup; consecutive draws mostly use the same pipeline.
	LastPipeline m_lastPipeline;

	// Cross queue synchronization.
	VkSemaphore m_timelineSemaphore = VK_NULL_HANDLE;
	uint64_t m_timelineSemaphoreValue = 0;

	// Stats.
	bool m_haveDebugMarkers = false;
	bool m_cursorVisible = true;
	int32_t m_firstQueryIndex = 0;
	int32_t m_nextQueryIndex = 0;
	int32_t m_lastQueryIndex = 0;
	uint32_t m_counter = -1;
	uint32_t m_passCount = 0;
	uint32_t m_drawCalls = 0;
	uint32_t m_primitiveCount = 0;

	bool create(uint32_t width, uint32_t height, uint32_t multiSample, float multiSampleShading, int32_t vblanks, bool allowHDR);

	bool validateGraphicsPipeline(const VertexLayoutVk* vertexLayout, const ProgramVk* program, PrimitiveType pt);

	bool validateComputePipeline(CommandBuffer* commandBuffer, const ProgramVk* p, bool asynchronous);

	//! Reserve (or return the already reserved) timeline value for the current frame's open asynchronous compute batch.
	uint64_t openComputeBatch(Frame& frame);

	//! Get a command buffer to continue recording the frame into after a split; reuses consumed command buffers.
	Ref< CommandBuffer > acquireFrameCommandBuffer(Frame& frame, bool compute);

	//! Submit the frame's compute command buffer signalling the timeline value, and continue in a fresh command buffer.
	bool splitCompute(Frame& frame, uint64_t value);

	//! Submit the frame's graphics command buffer waiting upon the timeline value, and continue in a fresh command buffer.
	bool splitGraphics(Frame& frame, uint64_t waitValue);

#if defined(_WIN32)
	// \name IWindowListener implementation.
	// \{

	virtual bool windowListenerEvent(Window* window, UINT message, WPARAM wParam, LPARAM lParam, LRESULT& outResult) override final;

	// \}
#endif
};

}
