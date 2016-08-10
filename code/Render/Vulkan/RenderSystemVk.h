#ifndef traktor_render_RenderSystemVk_H
#define traktor_render_RenderSystemVk_H

#define VK_USE_PLATFORM_WIN32_KHR
#define VK_NO_PROTOTYPES
#include <vulkan.h>

#include "Core/Ref.h"
#include "Render/IRenderSystem.h"

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

#if defined(_WIN32)
class Window;
#endif

/*! \brief Vulkan render system.
 * \ingroup Vulkan
 *
 * Vulkan render system implementation.
 */
class T_DLLCLASS RenderSystemVk : public IRenderSystem
{
	T_RTTI_CLASS;

public:
	RenderSystemVk();

	virtual bool create(const RenderSystemDesc& desc) T_OVERRIDE T_FINAL;

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual bool reset(const RenderSystemDesc& desc) T_OVERRIDE T_FINAL;

	virtual void getInformation(RenderSystemInformation& outInfo) const T_OVERRIDE T_FINAL;

	virtual uint32_t getDisplayModeCount() const T_OVERRIDE T_FINAL;
	
	virtual DisplayMode getDisplayMode(uint32_t index) const T_OVERRIDE T_FINAL;
	
	virtual DisplayMode getCurrentDisplayMode() const T_OVERRIDE T_FINAL;

	virtual float getDisplayAspectRatio() const T_OVERRIDE T_FINAL;

	virtual Ref< IRenderView > createRenderView(const RenderViewDefaultDesc& desc) T_OVERRIDE T_FINAL;

	virtual Ref< IRenderView > createRenderView(const RenderViewEmbeddedDesc& desc) T_OVERRIDE T_FINAL;

	virtual Ref< VertexBuffer > createVertexBuffer(const std::vector< VertexElement >& vertexElements, uint32_t bufferSize, bool dynamic) T_OVERRIDE T_FINAL;

	virtual Ref< IndexBuffer > createIndexBuffer(IndexType indexType, uint32_t bufferSize, bool dynamic) T_OVERRIDE T_FINAL;

	virtual Ref< ISimpleTexture > createSimpleTexture(const SimpleTextureCreateDesc& desc) T_OVERRIDE T_FINAL;

	virtual Ref< ICubeTexture > createCubeTexture(const CubeTextureCreateDesc& desc) T_OVERRIDE T_FINAL;
	
	virtual Ref< IVolumeTexture > createVolumeTexture(const VolumeTextureCreateDesc& desc) T_OVERRIDE T_FINAL;

	virtual Ref< RenderTargetSet > createRenderTargetSet(const RenderTargetSetCreateDesc& desc) T_OVERRIDE T_FINAL;

	virtual Ref< IProgram > createProgram(const ProgramResource* programResource, const wchar_t* const tag) T_OVERRIDE T_FINAL;

	virtual Ref< IProgramCompiler > createProgramCompiler() const T_OVERRIDE T_FINAL;

	virtual Ref< ITimeQuery > createTimeQuery() const T_OVERRIDE T_FINAL;

	virtual void purge() T_OVERRIDE T_FINAL;

	virtual void getStatistics(RenderSystemStatistics& outStatistics) const T_OVERRIDE T_FINAL;

private:
#if defined(_WIN32)
	Ref< Window > m_window;
#endif

	VkInstance m_vkInstance;
};

	}
}

#endif	// traktor_render_RenderSystemVk_H
