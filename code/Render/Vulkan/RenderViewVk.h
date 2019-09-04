#pragma once

#if defined(_WIN32)
#	define VK_USE_PLATFORM_WIN32_KHR
#	define VK_NO_PROTOTYPES
#	include <vulkan/vulkan.h>
#elif defined(__LINUX__)
#	define VK_USE_PLATFORM_LINUX_KHR
#	define VK_NO_PROTOTYPES
#	include <vulkan/vulkan.h>
#elif defined(__ANDROID__)
#	define VK_USE_PLATFORM_ANDROID_KHR
#	define VK_NO_PROTOTYPES
#	include <vulkan/vulkan.h>
#endif

#include <list>
#include <tuple>
#include "Core/Containers/AlignedVector.h"
#include "Render/IRenderView.h"
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

	virtual Viewport getViewport() override final;

	virtual SystemWindow getSystemWindow() override final;

	virtual bool begin(
		const Clear* clear
	) override final;

	virtual bool begin(
		RenderTargetSet* renderTargetSet,
		const Clear* clear
	) override final;

	virtual bool begin(
		RenderTargetSet* renderTargetSet,
		int32_t renderTarget,
		const Clear* clear
	) override final;

	virtual void draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives) override final;

	virtual void draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives, uint32_t instanceCount) override final;

	virtual void compute(IProgram* program, const int32_t* workSize) override final;

	virtual void end() override final;

	virtual void present() override final;

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
		Viewport viewport;
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
	Viewport m_viewport;
	AlignedVector< TargetState > m_targetStateStack;
	bool m_targetStateDirty;
	uint32_t m_targetId;
	VkRenderPass m_targetRenderPass;
	SmallMap< pipeline_key_t, VkPipeline > m_pipelines;
	Ref< UniformBufferPoolVk > m_uniformBufferPool;
	uint32_t m_drawCalls;
	uint32_t m_primitiveCount;

	bool create(uint32_t width, uint32_t height);

	void validateTargetState();

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
