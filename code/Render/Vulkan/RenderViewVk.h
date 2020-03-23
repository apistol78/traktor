#pragma once

#include <list>
#include <tuple>
#include "Core/System.h"
#include "Core/Containers/AlignedVector.h"
#include "Render/IRenderView.h"
#include "Render/Vulkan/ApiHeader.h"
#if defined(_WIN32)
#	include "Render/Vulkan/Win32/Window.h"
#elif defined(__LINUX__)
#	include "Render/Vulkan/Linux/Window.h"
#endif

namespace traktor
{
	namespace render
	{

class ProgramVk;
class RenderTargetSetVk;
class UniformBufferPoolVk;
class VertexBufferVk;

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
	RenderViewVk(
		VkInstance instance,
		VkPhysicalDevice physicalDevice,
		VkDevice device,
		VmaAllocator allocator,
		uint32_t graphicsQueueIndex,
		uint32_t computeQueueIndex
	);

	virtual ~RenderViewVk();

	bool create(const RenderViewDefaultDesc& desc);

	bool create(const RenderViewEmbeddedDesc& desc);

	virtual bool nextEvent(RenderEvent& outEvent) override final;

	virtual void close() override final;

	virtual bool reset(const RenderViewDefaultDesc& desc) override final;

	virtual bool reset(int32_t width, int32_t height) override final;

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

	virtual bool beginPass(const Clear* clear) override final;

	virtual bool beginPass(IRenderTargetSet* renderTargetSet, const Clear* clear) override final;

	virtual bool beginPass(IRenderTargetSet* renderTargetSet, int32_t renderTarget, const Clear* clear) override final;

	virtual void endPass() override final;

	virtual void draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives) override final;

	virtual void draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives, uint32_t instanceCount) override final;

	virtual void compute(IProgram* program, const int32_t* workSize) override final;

	virtual bool copy(ITexture* destinationTexture, const Region& destinationRegion, ITexture* sourceTexture, const Region& sourceRegion) override final;

	virtual void pushMarker(const char* const marker) override final;

	virtual void popMarker() override final;

	virtual void getStatistics(RenderViewStatistics& outStatistics) const override final;

	virtual bool getBackBufferContent(void* buffer) const override final;

private:
	typedef std::tuple< uint32_t, uint32_t, uint32_t, uint32_t > pipeline_key_t;

	struct TargetState
	{
		Ref< RenderTargetSetVk > rts;
		int32_t colorIndex;
		Clear clear;
	};

	VkInstance m_instance;
	VkPhysicalDevice m_physicalDevice;
	VkDevice m_logicalDevice;
	VmaAllocator m_allocator;
	uint32_t m_graphicsQueueIndex;
	uint32_t m_computeQueueIndex;
#if defined(_WIN32) || defined(__LINUX__)
	Ref< Window > m_window;
#endif
	VkSurfaceKHR m_surface;
	uint32_t m_presentQueueIndex;
	VkQueue m_presentQueue;
	VkCommandPool m_graphicsCommandPool;
	VkCommandPool m_computeCommandPool;
	VkCommandBuffer m_graphicsCommandBuffer;
	VkCommandBuffer m_computeCommandBuffer;
	VkSwapchainKHR m_swapChain;
	VkDescriptorPool m_descriptorPool;
	VkFence m_renderFence;
	VkSemaphore m_presentCompleteSemaphore;
	bool m_haveDebugMarkers;
	RefArray< RenderTargetSetVk > m_primaryTargets;
	std::list< RenderEvent > m_eventQueue;
	uint32_t m_currentImageIndex;
	TargetState m_targetState;
	uint32_t m_targetId;
	VkRenderPass m_targetRenderPass;
	VkFramebuffer m_targetFrameBuffer;
	SmallMap< pipeline_key_t, VkPipeline > m_pipelines;
	Ref< UniformBufferPoolVk > m_uniformBufferPool;
	bool m_cursorVisible;
	uint32_t m_drawCalls;
	uint32_t m_primitiveCount;

	bool create(uint32_t width, uint32_t height);

	bool validatePipeline(VertexBufferVk* vb, ProgramVk* p, PrimitiveType pt);

#if defined(_WIN32)
	// \name IWindowListener implementation.
	// \{

	virtual bool windowListenerEvent(Window* window, UINT message, WPARAM wParam, LPARAM lParam, LRESULT& outResult) override final;

	// \}
#endif
};

	}
}
