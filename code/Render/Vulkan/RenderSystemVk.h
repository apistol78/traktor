#pragma once

#include "Core/Ref.h"
#include "Render/IRenderSystem.h"
#include "Render/Vulkan/Private/ApiHeader.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_VULKAN_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class Context;
class PipelineLayoutCache;
class ShaderModuleCache;

#if defined(_WIN32) || defined(__LINUX__)
class Window;
#endif

/*! Vulkan render system.
 * \ingroup Vulkan
 *
 * Vulkan render system implementation.
 */
class T_DLLCLASS RenderSystemVk : public IRenderSystem
{
	T_RTTI_CLASS;

public:
	RenderSystemVk();

	virtual bool create(const RenderSystemDesc& desc) override final;

	virtual void destroy() override final;

	virtual bool reset(const RenderSystemDesc& desc) override final;

	virtual void getInformation(RenderSystemInformation& outInfo) const override final;

	virtual uint32_t getDisplayModeCount() const override final;

	virtual DisplayMode getDisplayMode(uint32_t index) const override final;

	virtual DisplayMode getCurrentDisplayMode() const override final;

	virtual float getDisplayAspectRatio() const override final;

	virtual Ref< IRenderView > createRenderView(const RenderViewDefaultDesc& desc) override final;

	virtual Ref< IRenderView > createRenderView(const RenderViewEmbeddedDesc& desc) override final;

	virtual Ref< VertexBuffer > createVertexBuffer(const AlignedVector< VertexElement >& vertexElements, uint32_t bufferSize, bool dynamic) override final;

	virtual Ref< IndexBuffer > createIndexBuffer(IndexType indexType, uint32_t bufferSize, bool dynamic) override final;

	virtual Ref< StructBuffer > createStructBuffer(const AlignedVector< StructElement >& structElements, uint32_t bufferSize) override final;

	virtual Ref< ISimpleTexture > createSimpleTexture(const SimpleTextureCreateDesc& desc, const wchar_t* const tag) override final;

	virtual Ref< ICubeTexture > createCubeTexture(const CubeTextureCreateDesc& desc, const wchar_t* const tag) override final;

	virtual Ref< IVolumeTexture > createVolumeTexture(const VolumeTextureCreateDesc& desc, const wchar_t* const tag) override final;

	virtual Ref< IRenderTargetSet > createRenderTargetSet(const RenderTargetSetCreateDesc& desc, IRenderTargetSet* sharedDepthStencil, const wchar_t* const tag) override final;

	virtual Ref< IProgram > createProgram(const ProgramResource* programResource, const wchar_t* const tag) override final;

	virtual void purge() override final;

	virtual void getStatistics(RenderSystemStatistics& outStatistics) const override final;

private:
#if defined(_WIN32) || defined(__LINUX__)
	Ref< Window > m_window;
#endif
#if defined(__LINUX__)
	void* m_display;
#endif
#if defined(__ANDROID__)
	int32_t m_screenWidth;
	int32_t m_screenHeight;
#endif
	VkInstance m_instance;
	VkPhysicalDevice m_physicalDevice;
	VkDevice m_logicalDevice;
	VkDebugUtilsMessengerEXT m_debugMessenger;
	Ref< Context > m_context;
	Ref< ShaderModuleCache > m_shaderModuleCache;
	Ref< PipelineLayoutCache > m_pipelineLayoutCache;
	VmaAllocator m_allocator;
	int32_t m_maxAnisotropy;
	float m_mipBias;
};

	}
}

