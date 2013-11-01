#ifndef traktor_render_RenderViewDx11_H
#define traktor_render_RenderViewDx11_H

#include <list>
#include <stack>
#include "Core/Misc/ComRef.h"
#include "Render/IRenderView.h"
#include "Render/Dx11/Platform.h"
#include "Render/Dx11/StateCache.h"
#include "Render/Dx11/Window.h"

namespace traktor
{
	namespace render
	{

class ContextDx11;
class VertexBufferDx11;
class IndexBufferDx11;
class ProgramDx11;
class RenderTargetDx11;
class RenderTargetSetDx11;

/*!
 * \ingroup DX11
 */
class RenderViewDx11
:	public IRenderView
,	public IWindowListener
{
	T_RTTI_CLASS;

public:
	RenderViewDx11(
		ContextDx11* context,
		Window* window
	);

	RenderViewDx11::RenderViewDx11(
		ContextDx11* context,
		IDXGISwapChain* dxgiSwapChain
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

	virtual void showCursor();

	virtual void hideCursor();

	virtual bool isCursorVisible() const;

	virtual bool setGamma(float gamma);

	virtual void setViewport(const Viewport& viewport);

	virtual Viewport getViewport();

	virtual SystemWindow getSystemWindow();

	virtual bool begin(EyeType eye);

	virtual bool begin(RenderTargetSet* renderTargetSet);

	virtual bool begin(RenderTargetSet* renderTargetSet, int renderTarget);

	virtual void clear(uint32_t clearMask, const Color4f* colors, float depth, int32_t stencil);

	virtual void draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives);

	virtual void draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives, uint32_t instanceCount);

	virtual void end();

	virtual void present();

	virtual void pushMarker(const char* const marker);

	virtual void popMarker();

	virtual void getStatistics(RenderViewStatistics& outStatistics) const;

	virtual bool getBackBufferContent(void* buffer) const;

private:
	struct RenderState
	{
		D3D11_VIEWPORT d3dViewport;
		RenderTargetSetDx11* renderTargetSet;
		RenderTargetDx11* renderTarget[2];
		ID3D11RenderTargetView* d3dRenderView[2];
		ID3D11DepthStencilView* d3dDepthStencilView;
		int32_t targetSize[2];
	};

	Ref< ContextDx11 > m_context;
	Ref< Window > m_window;
	ComRef< IDXGISwapChain > m_dxgiSwapChain;
	ComRef< ID3D11RenderTargetView > m_d3dRenderTargetView;
	ComRef< ID3D11Texture2D > m_d3dDepthStencil;
	ComRef< ID3D11DepthStencilView > m_d3dDepthStencilView;
	D3D11_VIEWPORT m_d3dViewport;
	StateCache m_stateCache;
	bool m_fullScreen;
	bool m_waitVBlank;
	bool m_cursorVisible;
	bool m_targetsDirty;
	uint32_t m_drawCalls;
	uint32_t m_primitiveCount;
	Ref< VertexBufferDx11 > m_currentVertexBuffer;
	Ref< IndexBufferDx11 > m_currentIndexBuffer;
	Ref< ProgramDx11 > m_currentProgram;
	std::list< RenderState > m_renderStateStack;
	int32_t m_targetSize[2];
	std::list< RenderEvent > m_eventQueue;

	void bindTargets();

	// \name IWindowListener implementation.
	// \{

	virtual bool windowListenerEvent(Window* window, UINT message, WPARAM wParam, LPARAM lParam, LRESULT& outResult);

	// \}
};

	}
}

#endif	// traktor_render_RenderViewDx11_H
