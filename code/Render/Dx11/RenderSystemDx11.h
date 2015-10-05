#ifndef traktor_render_RenderSystemDx11_H
#define traktor_render_RenderSystemDx11_H

#include "Core/RefArray.h"
#include "Core/Misc/AutoPtr.h"
#include "Core/Misc/ComRef.h"
#include "Render/IRenderSystem.h"
#include "Render/Dx11/Platform.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_DX11_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class ContextDx11;
class IBufferHeapDx11;
class ResourceCache;
class Window;

/*! \brief DirectX 10 render system.
 * \ingroup DX11
 *
 * OpenGL render system implementation.
 */
class T_DLLCLASS RenderSystemDx11 : public IRenderSystem
{
	T_RTTI_CLASS;

public:
	RenderSystemDx11();

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
	Ref< ContextDx11 > m_context;
	Ref< IBufferHeapDx11 > m_vertexBufferStaticHeap;
	Ref< IBufferHeapDx11 > m_indexBufferStaticHeap;
	Ref< Window > m_window;
	Ref< ResourceCache > m_resourceCache;
	bool m_resourceCreateLock;
	float m_displayAspect;
};

	}
}

#endif	// traktor_render_RenderSystemDx11_H
