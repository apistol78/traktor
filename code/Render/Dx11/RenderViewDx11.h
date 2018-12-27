/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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

	virtual Viewport getViewport() override final;

	virtual SystemWindow getSystemWindow() override final;

	virtual bool begin(EyeType eye) override final;

	virtual bool begin(RenderTargetSet* renderTargetSet) override final;

	virtual bool begin(RenderTargetSet* renderTargetSet, int renderTarget) override final;

	virtual void clear(uint32_t clearMask, const Color4f* colors, float depth, int32_t stencil) override final;

	virtual void draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives) override final;

	virtual void draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives, uint32_t instanceCount) override final;

	virtual void end() override final;

	virtual void present() override final;

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
		D3D11_VIEWPORT d3dViewport;
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
	D3D11_VIEWPORT m_d3dViewport;
	StateCache m_stateCache;
	bool m_fullScreen;
	int32_t m_waitVBlanks;
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
