#ifndef traktor_render_RenderSystemXbox360_H
#define traktor_render_RenderSystemXbox360_H

#include <list>
#include "Render/Dx9/Platform.h"
#include "Render/Dx9/Unmanaged.h"
#include "Render/RenderSystem.h"
#include "Core/Heap/Ref.h"
#include "Core/Misc/ComRef.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_DX9_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class ContextDx9;
class ParameterCache;
class VertexDeclCache;
class RenderTargetPool;
class RenderViewXbox360;

/*! \brief DirectX 9 (XBOX360) render system.
 * \ingroup Xbox360
 *
 * DX9 render system implementation.
 */
class T_DLLCLASS RenderSystemXbox360
:	public RenderSystem
,	public UnmanagedListener
{
	T_RTTI_CLASS(RenderSystemXbox360)

public:
	RenderSystemXbox360();

	virtual bool create();

	virtual void destroy();

	virtual int getDisplayModeCount() const;
	
	virtual DisplayMode* getDisplayMode(int index);
	
	virtual DisplayMode* getCurrentDisplayMode();

	virtual bool handleMessages();

	virtual RenderView* createRenderView(const DisplayMode* displayMode, const RenderViewCreateDesc& desc);

	virtual RenderView* createRenderView(void* windowHandle, const RenderViewCreateDesc& desc);

	virtual VertexBuffer* createVertexBuffer(const std::vector< VertexElement >& vertexElements, uint32_t bufferSize, bool dynamic);

	virtual IndexBuffer* createIndexBuffer(IndexType indexType, uint32_t bufferSize, bool dynamic);

	virtual SimpleTexture* createSimpleTexture(const SimpleTextureCreateDesc& desc);

	virtual CubeTexture* createCubeTexture(const CubeTextureCreateDesc& desc);
	
	virtual VolumeTexture* createVolumeTexture(const VolumeTextureCreateDesc& desc);

	virtual RenderTargetSet* createRenderTargetSet(const RenderTargetSetCreateDesc& desc);

	virtual ProgramResource* compileProgram(const ShaderGraph* shaderGraph, int optimize, bool validate);

	virtual Program* createProgram(const ProgramResource* programResource);

	/*! \name Unmanaged listener. */
	//@{

	virtual void addUnmanaged(Unmanaged* unmanaged);

	virtual void removeUnmanaged(Unmanaged* unmanaged);

	//@}

	/*! \name Direct3D interface. */
	//@{

	IDirect3D9* getD3D() const { return m_d3d; }

	//@}

private:
	Ref< ContextDx9 > m_context;
	ComRef< IDirect3D9 > m_d3d;
	ComRef< IDirect3DDevice9 > m_d3dDevice;
	ParameterCache* m_parameterCache;
	VertexDeclCache* m_vertexDeclCache;
	Ref< RenderTargetPool > m_renderTargetPool;
	Ref< RenderViewXbox360 > m_renderView;
	std::vector< ComRef< IDirect3DResource9 > > m_deferredRelease;
};

	}
}

#endif	// traktor_render_RenderSystemXbox360_H
