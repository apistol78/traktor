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

	virtual bool create(const RenderSystemDesc& desc);

	virtual void destroy();

	virtual bool reset(const RenderSystemDesc& desc);

	virtual void getInformation(RenderSystemInformation& outInfo) const;

	virtual uint32_t getDisplayModeCount() const;
	
	virtual DisplayMode getDisplayMode(uint32_t index) const;
	
	virtual DisplayMode getCurrentDisplayMode() const;

	virtual float getDisplayAspectRatio() const;

	virtual Ref< IRenderView > createRenderView(const RenderViewDefaultDesc& desc);

	virtual Ref< IRenderView > createRenderView(const RenderViewEmbeddedDesc& desc);

	virtual Ref< VertexBuffer > createVertexBuffer(const std::vector< VertexElement >& vertexElements, uint32_t bufferSize, bool dynamic);

	virtual Ref< IndexBuffer > createIndexBuffer(IndexType indexType, uint32_t bufferSize, bool dynamic);

	virtual Ref< ISimpleTexture > createSimpleTexture(const SimpleTextureCreateDesc& desc);

	virtual Ref< ICubeTexture > createCubeTexture(const CubeTextureCreateDesc& desc);
	
	virtual Ref< IVolumeTexture > createVolumeTexture(const VolumeTextureCreateDesc& desc);

	virtual Ref< RenderTargetSet > createRenderTargetSet(const RenderTargetSetCreateDesc& desc);

	virtual Ref< IProgram > createProgram(const ProgramResource* programResource, const wchar_t* const tag);

	virtual Ref< IProgramCompiler > createProgramCompiler() const;

	virtual Ref< ITimeQuery > createTimeQuery() const;

	virtual void purge();

	virtual void getStatistics(RenderSystemStatistics& outStatistics) const;

private:
	Ref< ContextDx11 > m_context;
	Ref< IBufferHeapDx11 > m_vertexBufferStaticHeap;
	Ref< IBufferHeapDx11 > m_indexBufferStaticHeap;
	Ref< Window > m_window;
	Ref< ResourceCache > m_resourceCache;
	float m_displayAspect;
};

	}
}

#endif	// traktor_render_RenderSystemDx11_H
