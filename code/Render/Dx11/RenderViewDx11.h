#pragma once

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

	virtual bool nextEvent(RenderEvent& outEvent) override final;

	virtual void close() override final;

	virtual bool reset(const RenderViewDefaultDesc& desc) override final;

	virtual bool reset(int32_t width, int32_t height) override final;

	virtual int getWidth() const override final;

	virtual int getHeight() const override final;

	virtual bool isActive() const override final;

	virtual bool isMinimized() const override final;

	virtual bool isFullScreen() const override final;

	virtual void showCursor() override final;

	virtual void hideCursor() override final;

	virtual bool isCursorVisible() const override final;

	virtual bool setGamma(float gamma) override final;

	virtual void setViewport(const Viewport& viewport) override final;

	virtual SystemWindow getSystemWindow() override final;

	virtual bool beginFrame() override final;

	virtual void endFrame() override final;

	virtual void present() override final;

	virtual bool beginPass(const Clear* clear) override final;

	virtual bool beginPass(IRenderTargetSet* renderTargetSet, const Clear* clear) override final;

	virtual bool beginPass(IRenderTargetSet* renderTargetSet, int32_t renderTarget, const Clear* clear) override final;

	virtual void endPass() override final;

	virtual void draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives) override final;

	virtual void draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives, uint32_t instanceCount) override final;

	virtual void compute(IProgram* program, const int32_t* workSize) override final;

	virtual bool copy(ITexture* destinationTexture, int32_t destinationSide, int32_t destinationLevel, ITexture* sourceTexture, int32_t sourceSide, int32_t sourceLevel) override final;

	virtual void pushMarker(const char* const marker) override final;

	virtual void popMarker() override final;

	virtual void getStatistics(RenderViewStatistics& outStatistics) const override final;

	virtual bool getBackBufferContent(void* buffer) const override final;

private:
	enum
	{
		TimeQueryFrames = 4,
		TimeQueryCount = 2048
	};

	struct RenderState
	{
		RenderTargetSetDx11* renderTargetSet;
		RenderTargetDx11* renderTarget[4];
		ID3D11RenderTargetView* d3dRenderView[4];
		ID3D11DepthStencilView* d3dDepthStencilView;
		int32_t targetSize[2];
	};

	Ref< ContextDx11 > m_context;
	Ref< Window > m_window;
	ComRef< IDXGISwapChain > m_dxgiSwapChain;
	ComRef< ID3D11RenderTargetView > m_d3dRenderTargetView;
	ComRef< ID3D11Texture2D > m_d3dDepthStencil;
	ComRef< ID3D11DepthStencilView > m_d3dDepthStencilView;
	StateCache m_stateCache;
	bool m_fullScreen;
	int32_t m_waitVBlanks;
	bool m_cursorVisible;
	uint32_t m_drawCalls;
	uint32_t m_primitiveCount;
	Ref< VertexBufferDx11 > m_currentVertexBuffer;
	Ref< IndexBufferDx11 > m_currentIndexBuffer;
	Ref< ProgramDx11 > m_currentProgram;
	RenderState m_renderState;
	int32_t m_targetSize[2];
	std::list< RenderEvent > m_eventQueue;

	// \name IWindowListener implementation.
	// \{

	virtual bool windowListenerEvent(Window* window, UINT message, WPARAM wParam, LPARAM lParam, LRESULT& outResult);

	// \}
};

	}
}
