#ifndef traktor_render_RenderViewDx11_H
#define traktor_render_RenderViewDx11_H

#include <list>
#include <stack>
#include "Core/Misc/ComRef.h"
#include "Render/IRenderView.h"
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
class VertexBufferDx11;
class IndexBufferDx11;
class ProgramDx11;
class RenderTargetSetDx11;
class Window;

/*!
 * \ingroup DX11
 */
class T_DLLCLASS RenderViewDx11 : public IRenderView
{
	T_RTTI_CLASS;

public:
	RenderViewDx11(
		ContextDx11* context,
		Window* window,
		IDXGISwapChain* d3dSwapChain,
		const DXGI_SWAP_CHAIN_DESC& scd,
		bool waitVBlank
	);

	virtual ~RenderViewDx11();

	virtual bool nextEvent(RenderEvent& outEvent);

	virtual void close();

	virtual bool reset(const RenderViewDefaultDesc& desc);

	virtual bool reset(int32_t width, int32_t height);

	virtual int getWidth() const;

	virtual int getHeight() const;

	virtual bool isActive() const;

	virtual bool isFullScreen() const;

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

private:
	Ref< ContextDx11 > m_context;
	ComRef< IDXGISwapChain > m_d3dSwapChain;
	ComRef< ID3D11RenderTargetView > m_d3dRenderTargetView;
	ComRef< ID3D11Texture2D > m_d3dDepthStencil;
	ComRef< ID3D11DepthStencilView > m_d3dDepthStencilView;
	bool m_waitVBlank;

	D3D11_VIEWPORT m_d3dViewport;

	bool m_dirty;
	Ref< VertexBufferDx11 > m_currentVertexBuffer;
	Ref< IndexBufferDx11 > m_currentIndexBuffer;
	Ref< ProgramDx11 > m_currentProgram;

	struct RenderState
	{
		D3D11_VIEWPORT d3dViewport;
		ID3D11RenderTargetView* d3dRenderView;
		ID3D11DepthStencilView* d3dDepthStencilView;
		int32_t targetSize[2];
	};

	std::list< RenderState > m_renderStateStack;
	int32_t m_targetSize[2];
};

	}
}

#endif	// traktor_render_RenderViewDx11_H
