#ifndef traktor_render_RenderViewXbox360_H
#define traktor_render_RenderViewXbox360_H

#include <map>
#include <stack>
#include "Core/Heap/Ref.h"
#include "Render/Dx9/Platform.h"
#include "Render/RenderView.h"
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

class RenderSystemXbox360;
class RenderTargetXbox360;
class RenderTargetPool;
class ProgramXbox360;
class VertexBufferDx9;
class IndexBufferDx9;

/*!
 * \ingroup Xbox360
 */
class T_DLLCLASS RenderViewXbox360 : public RenderView
{
	T_RTTI_CLASS(RenderViewXbox360)

public:
	RenderViewXbox360(
		const RenderViewCreateDesc& createDesc,
		RenderSystemXbox360* renderSystem,
		IDirect3DDevice9* d3dDevice,
		RenderTargetPool* renderTargetPool,
		int width,
		int height,
		D3DFORMAT d3dPrimaryTargetFormat
	);

	virtual ~RenderViewXbox360();

	virtual void close();

	virtual void resize(int32_t width, int32_t height);

	virtual void setViewport(const Viewport& viewport);

	virtual Viewport getViewport();

	virtual bool begin();

	virtual bool begin(RenderTargetSet* renderTargetSet, int renderTarget, bool keepDepthStencil);

	virtual void clear(uint32_t clearMask, const float color[4], float depth, int32_t stencil);

	virtual void setVertexBuffer(VertexBuffer* vertexBuffer);
	
	virtual void setIndexBuffer(IndexBuffer* indexBuffer);

	virtual void setProgram(Program* progrma);
	
	virtual void draw(const Primitives& primitives);

	virtual void end();

	virtual void present();

	IDirect3DDevice9* getD3DDevice() const { return m_d3dDevice; }

private:
	RenderViewCreateDesc m_createDesc;
	Ref< RenderSystemXbox360 > m_renderSystem;
	ComRef< IDirect3DDevice9 > m_d3dDevice;
	Ref< RenderTargetPool > m_renderTargetPool;
	Ref< RenderTargetXbox360 > m_renderTargetPrimary;

	struct RenderState
	{
		D3DVIEWPORT9 d3dViewport;
		RenderTargetXbox360* renderTarget;
	};

	std::list< RenderState > m_renderStateStack;
	RenderState* m_currentRenderState;

	D3DVIEWPORT9 m_d3dViewport;

	Ref< VertexBufferDx9 > m_currentVertexBuffer;
	Ref< IndexBufferDx9 > m_currentIndexBuffer;
	Ref< ProgramXbox360 > m_currentProgram;

	DWORD m_defaultGPR[3];
};

	}
}

#endif	// traktor_render_RenderViewXbox360_H
