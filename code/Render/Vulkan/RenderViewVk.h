#ifndef traktor_render_RenderViewVk_H
#define traktor_render_RenderViewVk_H

#define VK_USE_PLATFORM_WIN32_KHR
#define VK_NO_PROTOTYPES
#include <vulkan.h>

#include <list>
#include "Core/Containers/AlignedVector.h"
#include "Render/IRenderView.h"
#if defined(_WIN32)
#	include "Render/Vulkan/Win32/Window.h"
#endif

namespace traktor
{
	namespace render
	{

class ProgramVk;
class RenderTargetSetVk;
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
#if defined(_WIN32)
	RenderViewVk(
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
	);
#else
	RenderViewVk(VkDevice device);
#endif

	virtual ~RenderViewVk();

	virtual bool nextEvent(RenderEvent& outEvent) T_OVERRIDE T_FINAL;

	virtual void close() T_OVERRIDE T_FINAL;

	virtual bool reset(const RenderViewDefaultDesc& desc) T_OVERRIDE T_FINAL;

	virtual bool reset(int32_t width, int32_t height) T_OVERRIDE T_FINAL;

	virtual int getWidth() const T_OVERRIDE T_FINAL;

	virtual int getHeight() const T_OVERRIDE T_FINAL;

	virtual bool isActive() const T_OVERRIDE T_FINAL;

	virtual bool isMinimized() const T_OVERRIDE T_FINAL;

	virtual bool isFullScreen() const T_OVERRIDE T_FINAL;

	virtual void showCursor() T_OVERRIDE T_FINAL;

	virtual void hideCursor() T_OVERRIDE T_FINAL;

	virtual bool isCursorVisible() const T_OVERRIDE T_FINAL;

	virtual bool setGamma(float gamma) T_OVERRIDE T_FINAL;

	virtual void setViewport(const Viewport& viewport) T_OVERRIDE T_FINAL;

	virtual Viewport getViewport() T_OVERRIDE T_FINAL;

	virtual SystemWindow getSystemWindow() T_OVERRIDE T_FINAL;

	virtual bool begin(EyeType eye) T_OVERRIDE T_FINAL;

	virtual bool begin(RenderTargetSet* renderTargetSet) T_OVERRIDE T_FINAL;

	virtual bool begin(RenderTargetSet* renderTargetSet, int renderTarget) T_OVERRIDE T_FINAL;

	virtual void clear(uint32_t clearMask, const Color4f* colors, float depth, int32_t stencil) T_OVERRIDE T_FINAL;

	virtual void draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives) T_OVERRIDE T_FINAL;

	virtual void draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives, uint32_t instanceCount) T_OVERRIDE T_FINAL;

	virtual void end() T_OVERRIDE T_FINAL;

	virtual void present() T_OVERRIDE T_FINAL;

	virtual void pushMarker(const char* const marker) T_OVERRIDE T_FINAL;

	virtual void popMarker() T_OVERRIDE T_FINAL;

	virtual void getStatistics(RenderViewStatistics& outStatistics) const T_OVERRIDE T_FINAL;

	virtual bool getBackBufferContent(void* buffer) const T_OVERRIDE T_FINAL;

private:
#if defined(_WIN32)
	Ref< Window > m_window;
#endif
	VkDevice m_device;
#if defined(_WIN32)
	VkSwapchainKHR m_swapChain;
	VkQueue m_presentQueue;
	uint32_t m_currentImageIndex;
#endif
	VkCommandBuffer m_drawCmdBuffer;
	VkDescriptorSetLayout m_descriptorSetLayout;
	VkPipelineLayout m_pipelineLayout;
	VkDescriptorPool m_descriptorPool;
	VkDescriptorSet m_descriptorSet;
	RefArray< RenderTargetSetVk > m_primaryTargets;
	VkSemaphore m_presentCompleteSemaphore;
	VkSemaphore m_renderingCompleteSemaphore;
	std::list< RenderEvent > m_eventQueue;


	struct TargetState
	{
		Ref< RenderTargetSetVk > rts;
		int32_t colorIndex;
		uint32_t clearMask;
	};

	AlignedVector< TargetState > m_targetStateStack;
	bool m_targetStateDirty;

	VkPipeline m_pipeline;


	void validateTargetState();

	bool validatePipeline(VertexBufferVk* vb, ProgramVk* p, PrimitiveType pt);


#if defined(_WIN32)
	// \name IWindowListener implementation.
	// \{

	virtual bool windowListenerEvent(Window* window, UINT message, WPARAM wParam, LPARAM lParam, LRESULT& outResult) T_OVERRIDE T_FINAL;

	// \}
#endif
};

	}
}

#endif	// traktor_render_RenderViewVk_H
