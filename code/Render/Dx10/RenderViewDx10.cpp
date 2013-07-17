#include "Core/Log/Log.h"
#include "Render/Dx10/ContextDx10.h"
#include "Render/Dx10/IndexBufferDx10.h"
#include "Render/Dx10/ProgramDx10.h"
#include "Render/Dx10/RenderTargetDx10.h"
#include "Render/Dx10/RenderTargetSetDx10.h"
#include "Render/Dx10/RenderViewDx10.h"
#include "Render/Dx10/Utilities.h"
#include "Render/Dx10/VertexBufferDx10.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

const D3D10_PRIMITIVE_TOPOLOGY c_d3dTopology[] =
{
	D3D10_PRIMITIVE_TOPOLOGY_POINTLIST,
	D3D10_PRIMITIVE_TOPOLOGY_LINESTRIP,
	D3D10_PRIMITIVE_TOPOLOGY_LINELIST,
	D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,
	D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST
};

struct RenderEventTypePred
{
	RenderEventType m_type;

	RenderEventTypePred(RenderEventType type)
	:	m_type(type)
	{
	}

	bool operator () (const RenderEvent& evt) const
	{
		return evt.type == m_type;
	}
};
		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderViewDx10", RenderViewDx10, IRenderView)

RenderViewDx10::RenderViewDx10(
	ContextDx10* context,
	Window* window
)
:	m_context(context)
,	m_window(window)
,	m_fullScreen(false)
,	m_waitVBlank(true)
,	m_dirty(false)
,	m_drawCalls(0)
,	m_primitiveCount(0)
,	m_currentVertexBuffer(0)
,	m_currentIndexBuffer(0)
,	m_currentProgram(0)
{
	if (m_window)
		m_window->addListener(this);
}

RenderViewDx10::RenderViewDx10(
	ContextDx10* context,
	IDXGISwapChain* dxgiSwapChain
)
:	m_context(context)
,	m_dxgiSwapChain(dxgiSwapChain)
,	m_fullScreen(false)
,	m_waitVBlank(true)
,	m_dirty(false)
,	m_drawCalls(0)
,	m_primitiveCount(0)
,	m_currentVertexBuffer(0)
,	m_currentIndexBuffer(0)
,	m_currentProgram(0)
{
}

RenderViewDx10::~RenderViewDx10()
{
	close();
}

bool RenderViewDx10::nextEvent(RenderEvent& outEvent)
{
	MSG msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	if (!m_eventQueue.empty())
	{
		outEvent = m_eventQueue.front();
		m_eventQueue.pop_front();
		return true;
	}
	else
		return false;
}

void RenderViewDx10::close()
{
	if (m_window)
	{
		m_window->removeListener(this);
		m_window = 0;
	}

	m_d3dRenderTargetView.release();

	if (m_dxgiSwapChain)
	{
		m_dxgiSwapChain->SetFullscreenState(FALSE, NULL);
		m_dxgiSwapChain.release();
	}
}

bool RenderViewDx10::reset(const RenderViewDefaultDesc& desc)
{
	ComRef< ID3D10Texture2D > d3dBackBuffer;
	DXGI_SWAP_CHAIN_DESC scd;
	D3D10_TEXTURE2D_DESC dtd;
	D3D10_DEPTH_STENCIL_VIEW_DESC ddsvd;
	HRESULT hr;

	if (!m_window)
		return false;

	m_window->removeListener(this);

	m_context->getD3DDevice()->OMSetRenderTargets(0, NULL, NULL);

	if (m_dxgiSwapChain)
	{
		m_dxgiSwapChain->SetFullscreenState(FALSE, NULL);
		m_dxgiSwapChain.release();
	}

	m_d3dRenderTargetView.release();
	m_d3dDepthStencil.release();
	m_d3dDepthStencilView.release();

	if (desc.fullscreen)
	{
		m_window->setFullScreenStyle(desc.displayMode.width, desc.displayMode.height);

		std::memset(&scd, 0, sizeof(scd));
		scd.SampleDesc.Count = 1;
		scd.SampleDesc.Quality = 0;
		scd.BufferCount = 1;
		scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		scd.OutputWindow = *m_window;
		scd.Windowed = FALSE;

		if (!findDxgiDisplayMode(m_context->getDXGIOutput(), desc.displayMode, scd.BufferDesc))
		{
			log::error << L"Unable to create render view; display mode not supported" << Endl;
			return false;
		}
	}
	else
	{
		m_window->setWindowedStyle(desc.displayMode.width, desc.displayMode.height);

		std::memset(&scd, 0, sizeof(scd));
		scd.SampleDesc.Count = 1;
		scd.SampleDesc.Quality = 0;
		scd.BufferCount = 1;
		scd.BufferDesc.Width = desc.displayMode.width;
		scd.BufferDesc.Height = desc.displayMode.height;
		scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		scd.OutputWindow = *m_window;
		scd.Windowed = TRUE;
	}

	if (!setupSampleDesc(m_context->getD3DDevice(), desc.multiSample, scd.BufferDesc.Format, DXGI_FORMAT_D16_UNORM, scd.SampleDesc))
	{
		log::error << L"Unable to create render view; unsupported MSAA" << Endl;
		return false;
	}

	hr = m_context->getDXGIFactory()->CreateSwapChain(
		m_context->getD3DDevice(),
		&scd,
		&m_dxgiSwapChain.getAssign()
		);
	if (FAILED(hr))
	{
		log::error << L"Unable to create render view; CreateSwapChain failed" << Endl;
		return false;
	}

	hr = m_dxgiSwapChain->GetBuffer(0, __uuidof(ID3D10Texture2D), (void**)&d3dBackBuffer.getAssign());
	if (FAILED(hr))
	{
		log::error << L"Unable to create render view; GetBuffer failed" << Endl;
		return false;
	}

	hr = m_context->getD3DDevice()->CreateRenderTargetView(d3dBackBuffer, NULL, &m_d3dRenderTargetView.getAssign());
	if (FAILED(hr))
	{
		log::error << L"Unable to create render view; CreateRenderTargetView failed" << Endl;
		return false;
	}

	std::memset(&dtd, 0, sizeof(dtd));
	dtd.Width = scd.BufferDesc.Width;
	dtd.Height = scd.BufferDesc.Height;
	dtd.MipLevels = 1;
	dtd.ArraySize = 1;
	dtd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dtd.SampleDesc.Count = scd.SampleDesc.Count;
	dtd.SampleDesc.Quality = scd.SampleDesc.Quality;
	dtd.Usage = D3D10_USAGE_DEFAULT;
	dtd.BindFlags = D3D10_BIND_DEPTH_STENCIL;
	dtd.CPUAccessFlags = 0;
	dtd.MiscFlags = 0;

	hr = m_context->getD3DDevice()->CreateTexture2D(&dtd, NULL, &m_d3dDepthStencil.getAssign());
	if (FAILED(hr))
	{
		log::error << L"Unable to create render view; CreateTexture2D failed" << Endl;
		return false;
	}

	std::memset(&ddsvd, 0, sizeof(ddsvd));
	ddsvd.Format = dtd.Format;
	ddsvd.ViewDimension = dtd.SampleDesc.Count > 1 ? D3D10_DSV_DIMENSION_TEXTURE2DMS : D3D10_DSV_DIMENSION_TEXTURE2D;
	ddsvd.Texture2D.MipSlice = 0;

	hr = m_context->getD3DDevice()->CreateDepthStencilView(m_d3dDepthStencil, &ddsvd, &m_d3dDepthStencilView.getAssign());
	if (FAILED(hr))
	{
		log::error << L"Unable to create render view; CreateDepthStencilView failed" << Endl;
		return false;
	}

	m_d3dViewport.TopLeftX = 0;
	m_d3dViewport.TopLeftY = 0;
	m_d3dViewport.Width = scd.BufferDesc.Width;
	m_d3dViewport.Height = scd.BufferDesc.Height;
	m_d3dViewport.MinDepth = 0.0f;
	m_d3dViewport.MaxDepth = 1.0f;

	m_targetSize[0] = scd.BufferDesc.Width;
	m_targetSize[1] = scd.BufferDesc.Height;

	m_fullScreen = desc.fullscreen;
	m_waitVBlank = desc.waitVBlank;

	m_window->setTitle(!desc.title.empty() ? desc.title.c_str() : L"Traktor - DirectX 10 Renderer");
	m_window->addListener(this);

	return true;
}

bool RenderViewDx10::reset(int32_t width, int32_t height)
{
	ComRef< ID3D10Texture2D > d3dBackBuffer;
	DXGI_SWAP_CHAIN_DESC scd;
	D3D10_TEXTURE2D_DESC dtd;
	D3D10_DEPTH_STENCIL_VIEW_DESC ddsvd;
	HRESULT hr;

	if (m_window || width <= 0 || height <= 0)
		return false;

	m_context->getD3DDevice()->OMSetRenderTargets(0, NULL, NULL);

	m_d3dRenderTargetView.release();
	m_d3dDepthStencil.release();
	m_d3dDepthStencilView.release();

	hr = m_dxgiSwapChain->ResizeBuffers(1, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
	if (FAILED(hr))
	{
		log::error << L"Unable to reset render view; ResizeBuffers failed" << Endl;
		return false;
	}

	hr = m_dxgiSwapChain->GetBuffer(0, __uuidof(ID3D10Texture2D), (void**)&d3dBackBuffer.getAssign());
	if (FAILED(hr))
	{
		log::error << L"Unable to reset render view; GetBuffer failed" << Endl;
		return false;
	}

	hr = m_context->getD3DDevice()->CreateRenderTargetView(d3dBackBuffer, NULL, &m_d3dRenderTargetView.getAssign());
	if (FAILED(hr))
	{
		log::error << L"Unable to reset render view; CreateRenderTargetView failed" << Endl;
		return false;
	}

	m_dxgiSwapChain->GetDesc(&scd);

	std::memset(&dtd, 0, sizeof(dtd));
	dtd.Width = scd.BufferDesc.Width;
	dtd.Height = scd.BufferDesc.Height;
	dtd.MipLevels = 1;
	dtd.ArraySize = 1;
	dtd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dtd.SampleDesc.Count = scd.SampleDesc.Count;
	dtd.SampleDesc.Quality = scd.SampleDesc.Quality;
	dtd.Usage = D3D10_USAGE_DEFAULT;
	dtd.BindFlags = D3D10_BIND_DEPTH_STENCIL;
	dtd.CPUAccessFlags = 0;
	dtd.MiscFlags = 0;

	hr = m_context->getD3DDevice()->CreateTexture2D(&dtd, NULL, &m_d3dDepthStencil.getAssign());
	if (FAILED(hr))
	{
		log::error << L"Unable to reset render view; CreateTexture2D failed" << Endl;
		return false;
	}

	std::memset(&ddsvd, 0, sizeof(ddsvd));
	ddsvd.Format = dtd.Format;
	ddsvd.ViewDimension = dtd.SampleDesc.Count > 1 ? D3D10_DSV_DIMENSION_TEXTURE2DMS : D3D10_DSV_DIMENSION_TEXTURE2D;
	ddsvd.Texture2D.MipSlice = 0;

	hr = m_context->getD3DDevice()->CreateDepthStencilView(m_d3dDepthStencil, &ddsvd, &m_d3dDepthStencilView.getAssign());
	if (FAILED(hr))
	{
		log::error << L"Unable to reset render view; CreateDepthStencilView failed" << Endl;
		return false;
	}

	m_d3dViewport.TopLeftX = 0;
	m_d3dViewport.TopLeftY = 0;
	m_d3dViewport.Width = width;
	m_d3dViewport.Height = height;
	m_d3dViewport.MinDepth = 0.0f;
	m_d3dViewport.MaxDepth = 1.0f;

	m_targetSize[0] = width;
	m_targetSize[1] = height;

	return true;
}

int RenderViewDx10::getWidth() const
{
	return m_targetSize[0];
}

int RenderViewDx10::getHeight() const
{
	return m_targetSize[1];
}

bool RenderViewDx10::isActive() const
{
	if (m_window)
		return GetForegroundWindow() == *m_window;
	else
		return true;
}

bool RenderViewDx10::isFullScreen() const
{
	return m_fullScreen;
}

void RenderViewDx10::showCursor()
{
}

void RenderViewDx10::hideCursor()
{
}

bool RenderViewDx10::setGamma(float gamma)
{
	return false;
}

void RenderViewDx10::setViewport(const Viewport& viewport)
{
	if (m_renderStateStack.empty())
	{
		m_d3dViewport.TopLeftX = viewport.left;
		m_d3dViewport.TopLeftY = viewport.top;
		m_d3dViewport.Width = viewport.width;
		m_d3dViewport.Height = viewport.height;
		m_d3dViewport.MinDepth = viewport.nearZ;
		m_d3dViewport.MaxDepth = viewport.farZ;
	}
	else
	{
		D3D10_VIEWPORT& d3dViewport = m_renderStateStack.back().d3dViewport;

		d3dViewport.TopLeftX = viewport.left;
		d3dViewport.TopLeftY = viewport.top;
		d3dViewport.Width = viewport.width;
		d3dViewport.Height = viewport.height;
		d3dViewport.MinDepth = viewport.nearZ;
		d3dViewport.MaxDepth = viewport.farZ;

		m_context->getD3DDevice()->RSSetViewports(1, &d3dViewport);
	}
}

Viewport RenderViewDx10::getViewport()
{
	const D3D10_VIEWPORT& d3dViewport = m_renderStateStack.empty() ? m_d3dViewport : m_renderStateStack.back().d3dViewport;

	return Viewport(
		d3dViewport.TopLeftX,
		d3dViewport.TopLeftY,
		d3dViewport.Width,
		d3dViewport.Height,
		d3dViewport.MinDepth,
		d3dViewport.MaxDepth
	);
}

SystemWindow RenderViewDx10::getSystemWindow()
{
	SystemWindow sw;
	sw.hWnd = *m_window;
	return sw;
}

bool RenderViewDx10::begin(EyeType eye)
{
	T_ASSERT (m_renderStateStack.empty());

	if (!m_context->getD3DDevice())
		return false;

	RenderState rs =
	{
		m_d3dViewport,
		m_d3dRenderTargetView,
		m_d3dDepthStencilView,
		{ m_targetSize[0], m_targetSize[1] }
	};

	m_renderStateStack.push_back(rs);

	m_context->getD3DDevice()->OMSetRenderTargets(1, &rs.d3dRenderView, rs.d3dDepthStencilView);
	m_context->getD3DDevice()->RSSetViewports(1, &rs.d3dViewport);

	m_drawCalls = 0;
	m_primitiveCount = 0;

	return true;
}

bool RenderViewDx10::begin(RenderTargetSet* renderTargetSet)
{
	return false;
}

bool RenderViewDx10::begin(RenderTargetSet* renderTargetSet, int renderTarget)
{
	T_ASSERT (!m_renderStateStack.empty());

	if (!m_context)
		return false;

	RenderTargetSetDx10* rts = checked_type_cast< RenderTargetSetDx10* >(renderTargetSet);
	RenderTargetDx10* rt = checked_type_cast< RenderTargetDx10* >(rts->getColorTexture(renderTarget));
	RenderState rs =
	{
		{ 0, 0, rts->getWidth(), rts->getHeight(), 0.0f, 1.0f },
		rt->getD3D10RenderTargetView(),
		rts->getD3D10DepthTextureView(),
		{ rts->getWidth(), rts->getHeight() }
	};

	if (rts->usingPrimaryDepthStencil())
		rs.d3dDepthStencilView = m_d3dDepthStencilView;

	m_renderStateStack.push_back(rs);

	m_context->getD3DDevice()->OMSetRenderTargets(1, &rs.d3dRenderView, rs.d3dDepthStencilView);
	m_context->getD3DDevice()->RSSetViewports(1, &rs.d3dViewport);

	return true;
}

void RenderViewDx10::clear(uint32_t clearMask, const Color4f* color, float depth, int32_t stencil)
{
	T_ASSERT (!m_renderStateStack.empty());

	const RenderState& rs = m_renderStateStack.back();

	if (rs.d3dRenderView && (clearMask & CfColor) == CfColor)
	{
		float T_MATH_ALIGN16 tmp[4];
		color->storeAligned(tmp);
		m_context->getD3DDevice()->ClearRenderTargetView(rs.d3dRenderView, tmp);
	}

	if (rs.d3dDepthStencilView && (clearMask & CfDepth) == CfDepth)
		m_context->getD3DDevice()->ClearDepthStencilView(rs.d3dDepthStencilView, D3D10_CLEAR_DEPTH, depth, stencil);
}

void RenderViewDx10::draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives)
{
	T_ASSERT (!m_renderStateStack.empty());

	const RenderState& rs = m_renderStateStack.back();

	if (m_currentVertexBuffer != vertexBuffer)
	{
		m_currentVertexBuffer = checked_type_cast< VertexBufferDx10* >(vertexBuffer);
		m_dirty = true;
	}

	if (m_currentIndexBuffer != indexBuffer)
	{
		m_currentIndexBuffer = checked_type_cast< IndexBufferDx10* >(indexBuffer);
		m_dirty = true;
	}

	if (m_currentProgram != program)
	{
		m_currentProgram = checked_type_cast< ProgramDx10* >(program);
		m_dirty = true;
	}

	// Handle dirty resources.
	if (m_dirty)
	{
		// Bind vertex buffer.
		ID3D10Buffer* d3dBuffer = m_currentVertexBuffer->getD3D10Buffer();
		UINT stride = m_currentVertexBuffer->getD3D10Stride(), offset = 0;
		m_context->getD3DDevice()->IASetVertexBuffers(0, 1, &d3dBuffer, &stride, &offset);

		// Bind index buffer.
		if (m_currentIndexBuffer)
		{
			DXGI_FORMAT indexFormat = DXGI_FORMAT_UNKNOWN;

			switch (m_currentIndexBuffer->getIndexType())
			{
			case ItUInt16:
				indexFormat = DXGI_FORMAT_R16_UINT;
				break;

			case ItUInt32:
				indexFormat = DXGI_FORMAT_R32_UINT;
				break;
			}

			m_context->getD3DDevice()->IASetIndexBuffer(
				m_currentIndexBuffer->getD3D10Buffer(),
				indexFormat,
				0
			);
		}

		m_dirty = false;
	}

	// Bind program with device, handle input mapping of vertex elements.
	if (!m_currentProgram->bind(
		m_context->getD3DDevice(),
		size_t(m_currentVertexBuffer.ptr()),
		m_currentVertexBuffer->getD3D10InputElements(),
		rs.targetSize
	))
		return;

	// Draw primitives.
	T_ASSERT (c_d3dTopology[primitives.type] != D3D10_PRIMITIVE_TOPOLOGY_UNDEFINED);
	m_context->getD3DDevice()->IASetPrimitiveTopology(c_d3dTopology[primitives.type]);

	UINT vertexCount = 0;
	switch (primitives.type)
	{
	case PtPoints:
		vertexCount = primitives.count;
		break;

	case PtLineStrip:
		T_ASSERT (0);
		break;

	case PtLines:
		vertexCount = primitives.count * 2;
		break;

	case PtTriangleStrip:
		vertexCount = primitives.count + 2;
		break;

	case PtTriangles:
		vertexCount = primitives.count * 3;
		break;
	}

	if (primitives.indexed)
		m_context->getD3DDevice()->DrawIndexed(vertexCount, primitives.offset, 0);
	else
		m_context->getD3DDevice()->Draw(vertexCount, primitives.offset);

	m_drawCalls++;
	m_primitiveCount += primitives.count;
}

void RenderViewDx10::draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives, uint32_t instanceCount)
{
}

void RenderViewDx10::end()
{
	T_ASSERT (!m_renderStateStack.empty());

	m_renderStateStack.pop_back();
	if (!m_renderStateStack.empty())
	{
		const RenderState& rs = m_renderStateStack.back();

		m_context->getD3DDevice()->OMSetRenderTargets(1, &rs.d3dRenderView, rs.d3dDepthStencilView);
		m_context->getD3DDevice()->RSSetViewports(1, &rs.d3dViewport);
	}
}

void RenderViewDx10::present()
{
	m_dxgiSwapChain->Present(m_waitVBlank ? 1 : 0, 0);
	m_context->deleteResources();
}

void RenderViewDx10::pushMarker(const char* const marker)
{
}

void RenderViewDx10::popMarker()
{
}

void RenderViewDx10::getStatistics(RenderViewStatistics& outStatistics) const
{
	outStatistics.drawCalls = m_drawCalls;
	outStatistics.primitiveCount = m_primitiveCount;
	outStatistics.duration = 0.0;
}

bool RenderViewDx10::getBackBufferContent(void* buffer) const
{
	return false;
}

bool RenderViewDx10::windowListenerEvent(Window* window, UINT message, WPARAM wParam, LPARAM lParam, LRESULT& outResult)
{
	if (message == WM_CLOSE)
	{
		RenderEvent evt;
		evt.type = ReClose;
		m_eventQueue.push_back(evt);
	}
	else if (message == WM_SIZE)
	{
		// Remove all pending resize events.
		m_eventQueue.remove_if(RenderEventTypePred(ReResize));

		// Push new resize event if not matching current size.
		int32_t width = LOWORD(lParam);
		int32_t height = HIWORD(lParam);
		if (width != m_targetSize[0] || height != m_targetSize[1])
		{
			RenderEvent evt;
			evt.type = ReResize;
			evt.resize.width = width;
			evt.resize.height = height;
			m_eventQueue.push_back(evt);
		}
	}
	else if (message == WM_SIZING)
	{
		RECT* rcWindowSize = (RECT*)lParam;

		int32_t width = rcWindowSize->right - rcWindowSize->left;
		int32_t height = rcWindowSize->bottom - rcWindowSize->top;

		if (width < 320)
			width = 320;
		if (height < 200)
			height = 200;

		if (wParam == WMSZ_RIGHT || wParam == WMSZ_TOPRIGHT || wParam == WMSZ_BOTTOMRIGHT)
			rcWindowSize->right = rcWindowSize->left + width;
		if (wParam == WMSZ_LEFT || wParam == WMSZ_TOPLEFT || wParam == WMSZ_BOTTOMLEFT)
			rcWindowSize->left = rcWindowSize->right - width;

		if (wParam == WMSZ_BOTTOM || wParam == WMSZ_BOTTOMLEFT || wParam == WMSZ_BOTTOMRIGHT)
			rcWindowSize->bottom = rcWindowSize->top + height;
		if (wParam == WMSZ_TOP || wParam == WMSZ_TOPLEFT || wParam == WMSZ_TOPRIGHT)
			rcWindowSize->top = rcWindowSize->bottom - height;

		outResult = TRUE;
	}
	else if (message == WM_SYSKEYDOWN)
	{
		if (wParam == VK_RETURN && (lParam & (1 << 29)) != 0)
		{
			RenderEvent evt;
			evt.type = ReToggleFullScreen;
			m_eventQueue.push_back(evt);
		}
	}
	else if (message == WM_KEYDOWN)
	{
		if (wParam == VK_RETURN && (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0)
		{
			RenderEvent evt;
			evt.type = ReToggleFullScreen;
			m_eventQueue.push_back(evt);
		}
	}
	else if (message == WM_SETCURSOR)
	{
		if (isFullScreen())
			SetCursor(NULL);
		else
			SetCursor(LoadCursor(NULL, IDC_ARROW));
	}
	else
		return false;

	return true;
}

	}
}
