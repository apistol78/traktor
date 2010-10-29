#ifndef traktor_render_RenderViewDx10_H
#define traktor_render_RenderViewDx10_H

#include <list>
#include <stack>
#include "Core/Misc/ComRef.h"
#include "Render/IRenderView.h"
#include "Render/Dx10/Platform.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_DX10_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class ContextDx10;
class VertexBufferDx10;
class IndexBufferDx10;
class ProgramDx10;
class RenderTargetSetDx10;

/*!
 * \ingroup DX10
 */
class T_DLLCLASS RenderViewDx10 : public IRenderView
{
	T_RTTI_CLASS;

public:
	RenderViewDx10(
		ContextDx10* context,
		ID3D10Device* d3dDevice,
		IDXGISwapChain* d3dSwapChain,
		const DXGI_SWAP_CHAIN_DESC& scd,
		bool waitVBlank
	);

	virtual ~RenderViewDx10();

	virtual void close();

	virtual bool reset(const RenderViewDefaultDesc& desc);

	virtual void resize(int32_t width, int32_t height);

	virtual int getWidth() const;

	virtual int getHeight() const;

	virtual bool isActive() const;

	virtual bool isFullScreen() const;

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

private:
	Ref< ContextDx10 > m_context;
	ComRef< IDXGISwapChain > m_d3dSwapChain;
	ComRef< ID3D10Device > m_d3dDevice;
	ComRef< ID3D10RenderTargetView > m_d3dRenderTargetView;
	ComRef< ID3D10Texture2D > m_d3dDepthStencil;
	ComRef< ID3D10DepthStencilView > m_d3dDepthStencilView;
	bool m_waitVBlank;

	D3D10_VIEWPORT m_d3dViewport;

	bool m_dirty;
	Ref< VertexBufferDx10 > m_currentVertexBuffer;
	Ref< IndexBufferDx10 > m_currentIndexBuffer;
	Ref< ProgramDx10 > m_currentProgram;

	struct RenderState
	{
		D3D10_VIEWPORT d3dViewport;
		ID3D10RenderTargetView* d3dRenderView;
		ID3D10DepthStencilView* d3dDepthStencilView;
	};

	std::list< RenderState > m_renderStateStack;
};

	}
}

#endif	// traktor_render_RenderViewDx10_H
