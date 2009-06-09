#ifndef traktor_render_RenderSystemWin32_H
#define traktor_render_RenderSystemWin32_H

#include <list>
#include "Render/Dx9/Platform.h"
#include "Render/Dx9/Unmanaged.h"
#include "Render/RenderSystem.h"
#include "Core/Heap/Ref.h"
#include "Core/Thread/Semaphore.h"
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
:	public RenderSystem
,	public UnmanagedListener
{
	T_RTTI_CLASS(RenderSystemWin32)

public:
	RenderSystemWin32();

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
	RefList< RenderViewWin32 > m_renderViews;
	HWND m_hWnd;
	Semaphore m_unmanagedLock;
	std::list< Unmanaged* > m_unmanagedList;

	static LRESULT wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

	}
}

#endif	// traktor_render_RenderSystemWin32_H
