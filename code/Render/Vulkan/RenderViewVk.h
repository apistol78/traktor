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
class RenderPassCache;
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

	virtual int32_t beginTimeQuery() override final;

	virtual void endTimeQuery(int32_t query) override final;

	virtual bool getTimeQuery(int32_t query, bool wait, double& outStart, double& outEnd) const override final;

	virtual void pushMarker(bool asynchronous, const std::wstring& marker) override final;

	virtual void popMarker(bool asynchronous) override final;

	virtual void writeMarker(bool asynchronous, const std::wstring& marker) override final;

	virtual void getStatistics(RenderViewStatistics& outStatistics) const override final;

	Context* getContext() const { return m_context; }

	CommandBuffer* getGraphicsCommandBuffer();

private:
	struct Frame
	{
		Ref< CommandBuffer > graphicsCommandBuffer;
		Ref< CommandBuffer > computeCommandBuffer;
		VkSemaphore renderFinishedSemaphore;
		VkSemaphore computeFinishedSemaphore;
		Ref< RenderTargetSetVk > primaryTarget;
		VkPipeline boundGraphicsPipeline = 0;
		VkPipeline boundComputePipeline = 0;
		VkPipeline boundAsyncComputePipeline = 0;
		BufferViewVk boundIndexBuffer;
		BufferViewVk boundVertexBuffer;
		RefArray< CommandBuffer > flyingCommandBuffers;
		std::list< std::string > markers;
		AlignedVector< bool > markerStack;
		uint64_t computeRecordValue = 0;	//!< Timeline value of the open (not yet submitted) asynchronous compute batch; 0 if none open.
		uint64_t computeSubmittedValue = 0;	//!< Highest asynchronous compute batch value already submitted to the compute queue this frame.
	};

	Context* m_context = nullptr;
#if defined(_WIN32) || defined(__LINUX__) || defined(__RPI__) || defined(__MAC__)
	Ref< Window > m_window;
#endif
	VkSurfaceKHR m_surface = 0;
	Ref< Queue > m_presentQueue;
#if !defined(__ANDROID__) && !defined(__IOS__)
	VkQueryPool m_queryPool = 0;
#endif
	bool m_lost = true;

	VkPhysicalDeviceProperties m_deviceProperties;

	// Swap chain.
	VkSwapchainKHR m_swapChain = 0;

	// Pool of binary semaphores for vkAcquireNextImageKHR. We hold imageCount+1
	// semaphores: one in m_imageAvailableSemaphores[image_index] for each swap
	// chain image (the one consumed by the submit that produced that image's
	// content) plus a single spare passed to the next acquire. After each
	// acquire, the per-image slot and the spare are swapped — see beginFrame.
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

	// Cached surface state, populated lazily on first create() and kept across reset().
	bool m_surfaceCacheValid = false;
	VkFormat m_colorFormat = VK_FORMAT_UNDEFINED;
	VkColorSpaceKHR m_colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	VkPresentModeKHR m_presentMode = VK_PRESENT_MODE_FIFO_KHR;
	uint32_t m_presentQueueFamilyIndex = ~0u;

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

	// Cross queue synchronization.
	VkSemaphore m_timelineSemaphore = VK_NULL_HANDLE;
	uint64_t m_timelineSemaphoreValue = 0;

	// Stats.
	bool m_haveDebugMarkers = false;
	bool m_cursorVisible = true;
	int32_t m_nextQueryIndex = 0;
	int32_t m_lastQueryIndex = 0;
	AlignedVector< int32_t > m_openTimeQueries;
	uint32_t m_counter = -1;
	uint32_t m_passCount = 0;
	uint32_t m_drawCalls = 0;
	uint32_t m_primitiveCount = 0;

	bool create(uint32_t width, uint32_t height, uint32_t multiSample, float multiSampleShading, int32_t vblanks, bool allowHDR);

	bool validateGraphicsPipeline(const VertexLayoutVk* vertexLayout, const ProgramVk* program, PrimitiveType pt);

	bool validateComputePipeline(CommandBuffer* commandBuffer, const ProgramVk* p, bool asynchronous);

	//! Reserve (or return the already reserved) timeline value for the current frame's open asynchronous compute batch.
	uint64_t openComputeBatch(Frame& frame);

	//! Re-record time query resets into the fresh graphics command buffer after a mid-frame queue split.
	void rerecordTimeQueryReset(Frame& frame);

#if defined(_WIN32)
	// \name IWindowListener implementation.
	// \{

	virtual bool windowListenerEvent(Window* window, UINT message, WPARAM wParam, LPARAM lParam, LRESULT& outResult) override final;

	// \}
#endif
};

}
