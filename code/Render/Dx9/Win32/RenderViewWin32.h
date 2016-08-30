#ifndef traktor_render_RenderViewWin32_H
#define traktor_render_RenderViewWin32_H

#include <list>
#include <map>
#include <stack>
#include "Core/Misc/ComRef.h"
#include "Render/IRenderView.h"
#include "Render/Dx9/Platform.h"

namespace traktor
{
	namespace render
	{

class ClearTarget;
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
class RenderViewWin32
:	public IRenderView
{
	T_RTTI_CLASS;

public:
	RenderViewWin32(
		RenderSystemWin32* renderSystem,
		ClearTarget* clearTarget,
		ParameterCache* parameterCache,
		IDirect3DDevice9* d3dDevice
	);

	virtual void close() T_OVERRIDE;

	virtual bool setGamma(float gamma) T_OVERRIDE T_FINAL;

	virtual void setViewport(const Viewport& viewport) T_OVERRIDE T_FINAL;

	virtual Viewport getViewport() T_OVERRIDE T_FINAL;

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
		IDirect3DSurface9* d3dColorBuffer[2];
		IDirect3DSurface9* d3dDepthStencilSurface;
		RenderTargetWin32* renderTarget[2];
	};

	Ref< RenderSystemWin32 > m_renderSystem;
	ParameterCache* m_parameterCache;
	ClearTarget* m_clearTarget;
	ComRef< IDirect3DDevice9 > m_d3dDevice;
	ComRef< IDirect3DSwapChain9 > m_d3dSwapChain;
	ComRef< IDirect3DSurface9 > m_d3dBackBuffer;
	ComRef< IDirect3DSurface9 > m_d3dDepthStencilSurface;
	ComRef< IDirect3DQuery9 > m_d3dSyncQueries[1];
	D3DVIEWPORT9 m_d3dViewport;
	std::list< RenderState > m_renderStateStack;
	uint32_t m_frameCount;
	bool m_targetDirty;
	uint32_t m_drawCalls;
	uint32_t m_primitiveCount;

	void bindTargets();
};

	}
}

#endif	// traktor_render_RenderViewWin32_H
