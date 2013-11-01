#ifndef traktor_render_RenderViewDx10_H
#define traktor_render_RenderViewDx10_H

#include <list>
#include <stack>
#include "Core/Misc/ComRef.h"
#include "Render/IRenderView.h"
#include "Render/Dx10/Platform.h"
#include "Render/Dx11/Window.h"

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
class RenderViewDx10
:	public IRenderView
,	public IWindowListener
{
	T_RTTI_CLASS;

public:
	RenderViewDx10(
		ContextDx10* context,
		Window* window
	);

	RenderViewDx10(
		ContextDx10* context,
		IDXGISwapChain* dxgiSwapChain
	);

	virtual ~RenderViewDx10();

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

	virtual void clear(uint32_t clearMask, const Color4f* color, float depth, int32_t stencil);

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
		D3D10_VIEWPORT d3dViewport;
		ID3D10RenderTargetView* d3dRenderView;
		ID3D10DepthStencilView* d3dDepthStencilView;
		int32_t targetSize[2];
	};

	Ref< ContextDx10 > m_context;
	Ref< Window > m_window;
	ComRef< IDXGISwapChain > m_dxgiSwapChain;
	ComRef< ID3D10Device > m_d3dDevice;
	ComRef< ID3D10RenderTargetView > m_d3dRenderTargetView;
	ComRef< ID3D10Texture2D > m_d3dDepthStencil;
	ComRef< ID3D10DepthStencilView > m_d3dDepthStencilView;
	D3D10_VIEWPORT m_d3dViewport;
	bool m_fullScreen;
	bool m_waitVBlank;
	bool m_dirty;
	uint32_t m_drawCalls;
	uint32_t m_primitiveCount;
	Ref< VertexBufferDx10 > m_currentVertexBuffer;
	Ref< IndexBufferDx10 > m_currentIndexBuffer;
	Ref< ProgramDx10 > m_currentProgram;
	std::list< RenderState > m_renderStateStack;
	int32_t m_targetSize[2];
	std::list< RenderEvent > m_eventQueue;

	// \name IWindowListener implementation.
	// \{

	virtual bool windowListenerEvent(Window* window, UINT message, WPARAM wParam, LPARAM lParam, LRESULT& outResult);

	// \}
};

	}
}

#endif	// traktor_render_RenderViewDx10_H
