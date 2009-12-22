#ifndef traktor_render_RenderViewWin32_H
#define traktor_render_RenderViewWin32_H

#include <map>
#include <stack>
#include "Render/Dx9/Platform.h"
#include "Render/Dx9/Unmanaged.h"
#include "Render/IRenderView.h"
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

class RenderSystemWin32;
class RenderTargetSetWin32;
class RenderTargetWin32;
class ContextDx9;
class ProgramWin32;
class VertexBufferDx9;
class IndexBufferDx9;

/*!
 * \ingroup DX9
 */
class T_DLLCLASS RenderViewWin32
:	public IRenderView
,	public Unmanaged
{
	T_RTTI_CLASS;

public:
	RenderViewWin32(
		ContextDx9* context,
		const RenderViewCreateDesc& createDesc,
		RenderSystemWin32* renderSystem,
		const D3DPRESENT_PARAMETERS& d3dPresent,
		D3DFORMAT d3dDepthStencilFormat,
		float nativeAspectRatio
	);

	virtual ~RenderViewWin32();

	virtual void close();

	virtual void resize(int32_t width, int32_t height);

	virtual void setViewport(const Viewport& viewport);

	virtual Viewport getViewport();

	virtual bool getNativeAspectRatio(float& outAspectRatio) const;

	virtual bool begin();

	virtual bool begin(RenderTargetSet* renderTargetSet, int renderTarget, bool keepDepthStencil);

	virtual void clear(uint32_t clearMask, const float color[4], float depth, int32_t stencil);

	virtual void setVertexBuffer(VertexBuffer* vertexBuffer);
	
	virtual void setIndexBuffer(IndexBuffer* indexBuffer);

	virtual void setProgram(IProgram* program);
	
	virtual void draw(const Primitives& primitives);

	virtual void end();

	virtual void present();

	void setD3DBuffers(IDirect3DSwapChain9* d3dSwapChain, IDirect3DSurface9* d3dDepthStencilSurface);

	inline IDirect3DDevice9* getD3DDevice() const {
		return m_d3dDevice;
	}

	inline const D3DPRESENT_PARAMETERS& getD3DPresent() const {
		return m_d3dPresent;
	}

	inline D3DFORMAT getD3DDepthStencilFormat() const {
		return m_d3dDepthStencilFormat;
	}

protected:
	virtual HRESULT lostDevice();

	virtual HRESULT resetDevice(IDirect3DDevice9* d3dDevice);

private:
	struct RenderState
	{
		D3DVIEWPORT9 d3dViewport;
		IDirect3DSurface9* d3dBackBuffer;
		IDirect3DSurface9* d3dDepthStencilSurface;
		RenderTargetWin32* renderTarget;
	};

	Ref< ContextDx9 > m_context;
	RenderViewCreateDesc m_createDesc;
	Ref< RenderSystemWin32 > m_renderSystem;
	IDirect3DDevice9* m_d3dDevice;
	D3DPRESENT_PARAMETERS m_d3dPresent;
	D3DFORMAT m_d3dDepthStencilFormat;
	float m_nativeAspectRatio;
	D3DVIEWPORT9 m_d3dViewport;
	ComRef< IDirect3DSwapChain9 > m_d3dSwapChain;
	ComRef< IDirect3DSurface9 > m_d3dBackBuffer;
	ComRef< IDirect3DSurface9 > m_d3dDepthStencilSurface;
	std::list< RenderState > m_renderStateStack;
	Ref< VertexBufferDx9 > m_currentVertexBuffer;
	Ref< IndexBufferDx9 > m_currentIndexBuffer;
	Ref< ProgramWin32 > m_currentProgram;

#if defined(_DEBUG)
	LONG m_ownerThread;
	uint32_t m_frameCount;
	uint32_t m_drawCalls;
#endif
};

	}
}

#endif	// traktor_render_RenderViewWin32_H
