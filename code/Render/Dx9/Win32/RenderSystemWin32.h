#ifndef traktor_render_RenderSystemWin32_H
#define traktor_render_RenderSystemWin32_H

#include <list>
#include "Core/RefArray.h"
#include "Core/Misc/ComRef.h"
#include "Core/Thread/Semaphore.h"
#include "Render/IRenderSystem.h"
#include "Render/Dx9/Platform.h"
#include "Render/Dx9/Unmanaged.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_DX9_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class RenderViewWin32;
class ContextDx9;
class ShaderCache;
class ParameterCache;
class VertexDeclCache;

/*! \brief DirectX 9 render system.
 * \ingroup DX9
 *
 * DX9 render system implementation.
 */
class T_DLLCLASS RenderSystemWin32
:	public IRenderSystem
,	public UnmanagedListener
{
	T_RTTI_CLASS;

public:
	RenderSystemWin32();

	virtual bool create(const RenderSystemCreateDesc& desc);

	virtual void destroy();

	virtual int getDisplayModeCount() const;
	
	virtual Ref< DisplayMode > getDisplayMode(int index);
	
	virtual Ref< DisplayMode > getCurrentDisplayMode();

	virtual bool handleMessages();

	virtual Ref< IRenderView > createRenderView(const DisplayMode* displayMode, const RenderViewCreateDesc& desc);

	virtual Ref< IRenderView > createRenderView(void* windowHandle, const RenderViewCreateDesc& desc);

	virtual Ref< VertexBuffer > createVertexBuffer(const std::vector< VertexElement >& vertexElements, uint32_t bufferSize, bool dynamic);

	virtual Ref< IndexBuffer > createIndexBuffer(IndexType indexType, uint32_t bufferSize, bool dynamic);

	virtual Ref< ISimpleTexture > createSimpleTexture(const SimpleTextureCreateDesc& desc);

	virtual Ref< ICubeTexture > createCubeTexture(const CubeTextureCreateDesc& desc);
	
	virtual Ref< IVolumeTexture > createVolumeTexture(const VolumeTextureCreateDesc& desc);

	virtual Ref< RenderTargetSet > createRenderTargetSet(const RenderTargetSetCreateDesc& desc);

	virtual Ref< IProgram > createProgram(const ProgramResource* programResource);

	virtual Ref< IProgramCompiler > createProgramCompiler() const;

	/*! \name Direct3D interface. */
	//@{

	IDirect3D9* getD3D() const;

	IDirect3DDevice9* getD3DDevice() const;

	HRESULT testCooperativeLevel();

	HRESULT resetDevice();

	//@}

	/*! \name Render view management. */
	//@{

	void addRenderView(RenderViewWin32* renderView);

	void removeRenderView(RenderViewWin32* renderView);

	//@}

	/*! \name Unmanaged listener. */
	//@{

	virtual void addUnmanaged(Unmanaged* unmanaged);

	virtual void removeUnmanaged(Unmanaged* unmanaged);

	//@}

private:
	Ref< ContextDx9 > m_context;
	ComRef< IDirect3D9 > m_d3d;
	ComRef< IDirect3DDevice9 > m_d3dDevice;
	D3DDISPLAYMODE m_d3dDefaultDisplayMode;
	Ref< ShaderCache > m_shaderCache;
	ParameterCache* m_parameterCache;
	VertexDeclCache* m_vertexDeclCache;
	RefArray< RenderViewWin32 > m_renderViews;
	HWND m_hWnd;
	Semaphore m_unmanagedLock;
	std::list< Unmanaged* > m_unmanagedList;
	float m_mipBias;

	static LRESULT wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

	}
}

#endif	// traktor_render_RenderSystemWin32_H
