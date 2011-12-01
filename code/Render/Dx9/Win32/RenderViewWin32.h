#ifndef traktor_render_RenderViewWin32_H
#define traktor_render_RenderViewWin32_H

#include <list>
#include <map>
#include <stack>
#include "Core/Misc/ComRef.h"
#include "Render/IRenderView.h"
#include "Render/Dx9/Platform.h"

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

class IndexBufferDx9;
class ParameterCache;
class ProgramWin32;
class RenderSystemWin32;
class RenderTargetSetWin32;
class RenderTargetWin32;
class VertexBufferDx9;

/*!
 * \ingroup DX9
 */
class T_DLLCLASS RenderViewWin32
:	public IRenderView
{
	T_RTTI_CLASS;

public:
	RenderViewWin32(
		RenderSystemWin32* renderSystem,
		ParameterCache* parameterCache,
		IDirect3DDevice9* d3dDevice
	);

	virtual void close();

	virtual bool setGamma(float gamma);

	virtual void setViewport(const Viewport& viewport);

	virtual Viewport getViewport();

	virtual bool begin(EyeType eye);

	virtual bool begin(RenderTargetSet* renderTargetSet, int renderTarget);

	virtual void clear(uint32_t clearMask, const float color[4], float depth, int32_t stencil);

	virtual void setVertexBuffer(VertexBuffer* vertexBuffer);
	
	virtual void setIndexBuffer(IndexBuffer* indexBuffer);

	virtual void setProgram(IProgram* program);
	
	virtual void draw(const Primitives& primitives);

	virtual void end();

	virtual void present();

	virtual void pushMarker(const char* const marker);

	virtual void popMarker();

	virtual void getStatistics(RenderViewStatistics& outStatistics) const;

	// \name Swap-chain management
	// \{

	virtual HRESULT lostDevice() = 0;

	virtual HRESULT resetDevice() = 0;

	// \}

protected:
	struct RenderState
	{
		D3DVIEWPORT9 d3dViewport;
		int32_t targetSize[2];
		IDirect3DSurface9* d3dBackBuffer;
		IDirect3DSurface9* d3dDepthStencilSurface;
		RenderTargetWin32* renderTarget;
	};

	Ref< RenderSystemWin32 > m_renderSystem;
	ParameterCache* m_parameterCache;
	ComRef< IDirect3DDevice9 > m_d3dDevice;
	ComRef< IDirect3DSwapChain9 > m_d3dSwapChain;
	ComRef< IDirect3DSurface9 > m_d3dBackBuffer;
	ComRef< IDirect3DSurface9 > m_d3dDepthStencilSurface;
	ComRef< IDirect3DQuery9 > m_d3dSyncQueries[1];
	D3DVIEWPORT9 m_d3dViewport;
	std::list< RenderState > m_renderStateStack;
	Ref< VertexBufferDx9 > m_currentVertexBuffer;
	Ref< IndexBufferDx9 > m_currentIndexBuffer;
	Ref< ProgramWin32 > m_currentProgram;
	uint32_t m_frameCount;
	bool m_targetDirty;
	uint32_t m_drawCalls;
	uint32_t m_primitiveCount;

	void bindTargets();
};

	}
}

#endif	// traktor_render_RenderViewWin32_H
