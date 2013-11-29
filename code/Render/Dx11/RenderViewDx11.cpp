#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Core/Thread/Acquire.h"
#include "Render/Dx11/RenderViewDx11.h"
#include "Render/Dx11/ContextDx11.h"
#include "Render/Dx11/VertexBufferDx11.h"
#include "Render/Dx11/IndexBufferDx11.h"
#include "Render/Dx11/ProgramDx11.h"
#include "Render/Dx11/RenderTargetSetDx11.h"
#include "Render/Dx11/RenderTargetDx11.h"
#include "Render/Dx11/Utilities.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

const D3D11_PRIMITIVE_TOPOLOGY c_d3dTopology[] =
{
	D3D11_PRIMITIVE_TOPOLOGY_POINTLIST,
	D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP,
	D3D11_PRIMITIVE_TOPOLOGY_LINELIST,
	D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,
	D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST
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

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderViewDx11", RenderViewDx11, IRenderView)

RenderViewDx11::RenderViewDx11(
	ContextDx11* context,
	Window* window
)
:	m_context(context)
,	m_window(window)
,	m_stateCache(context->getD3DDeviceContext())
,	m_fullScreen(false)
,	m_waitVBlank(true)
,	m_cursorVisible(true)
,	m_targetsDirty(false)
,	m_drawCalls(0)
,	m_primitiveCount(0)
,	m_currentVertexBuffer(0)
,	m_currentIndexBuffer(0)
,	m_currentProgram(0)
{
	if (m_window)
		m_window->addListener(this);
}

RenderViewDx11::RenderViewDx11(
	ContextDx11* context,
	IDXGISwapChain* dxgiSwapChain
)
:	m_context(context)
,	m_dxgiSwapChain(dxgiSwapChain)
,	m_stateCache(context->getD3DDeviceContext())
,	m_fullScreen(false)
,	m_waitVBlank(true)
,	m_cursorVisible(true)
,	m_targetsDirty(false)
,	m_currentVertexBuffer(0)
,	m_currentIndexBuffer(0)
,	m_currentProgram(0)
{
}

RenderViewDx11::~RenderViewDx11()
{
	close();
}

bool RenderViewDx11::nextEvent(RenderEvent& outEvent)
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

void RenderViewDx11::close()
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

bool RenderViewDx11::reset(const RenderViewDefaultDesc& desc)
{
	ComRef< ID3D11Texture2D > d3dBackBuffer;
	DXGI_SWAP_CHAIN_DESC scd;
	D3D11_TEXTURE2D_DESC dtd;
	D3D11_DEPTH_STENCIL_VIEW_DESC ddsvd;
	HRESULT hr;

	if (!m_window)
		return false;

	DisplayMode dm;
	dm = desc.displayMode;

	// Ensure somewhat sane values.
	if (dm.width <= 0 || dm.height <= 0)
	{
		dm.width = 640;
		dm.height = 480;
	}

	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_context->getLock());

	m_window->removeListener(this);
	m_eventQueue.clear();

	m_context->getD3DDeviceContext()->OMSetRenderTargets(0, NULL, NULL);

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
		std::memset(&scd, 0, sizeof(scd));
		scd.SampleDesc.Count = 1;
		scd.SampleDesc.Quality = 0;
		scd.BufferCount = 1;
		scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		scd.OutputWindow = *m_window;
		scd.Windowed = FALSE;

		if (!findDxgiDisplayMode(m_context->getDXGIOutput(), dm, scd.BufferDesc))
		{
			dm.refreshRate = 0;
			if (!findDxgiDisplayMode(m_context->getDXGIOutput(), dm, scd.BufferDesc))
			{
				log::error << L"Unable to create render view; display mode not supported" << Endl;
				return false;
			}
		}

		m_window->setFullScreenStyle(scd.BufferDesc.Width, scd.BufferDesc.Height);
	}
	else
	{
		std::memset(&scd, 0, sizeof(scd));
		scd.SampleDesc.Count = 1;
		scd.SampleDesc.Quality = 0;
		scd.BufferCount = 1;
		scd.BufferDesc.Width = dm.width;
		scd.BufferDesc.Height = dm.height;
		scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		scd.OutputWindow = *m_window;
		scd.Windowed = TRUE;

		m_window->setWindowedStyle(scd.BufferDesc.Width, scd.BufferDesc.Height);
	}

	if (!setupSampleDesc(m_context->getD3DDevice(), desc.multiSample, scd.BufferDesc.Format, DXGI_FORMAT_D24_UNORM_S8_UINT, scd.SampleDesc))
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
		log::error << L"Unable to create render view; CreateSwapChain failed, HRESULT " << int32_t(hr) << Endl;
		return false;
	}

	m_context->getDXGIFactory()->MakeWindowAssociation(
		*m_window,
		DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_ALT_ENTER
	);

	hr = m_dxgiSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&d3dBackBuffer.getAssign());
	if (FAILED(hr))
	{
		log::error << L"Unable to create render view; GetBuffer failed, HRESULT " << int32_t(hr) << Endl;
		return false;
	}

	hr = m_context->getD3DDevice()->CreateRenderTargetView(d3dBackBuffer, NULL, &m_d3dRenderTargetView.getAssign());
	if (FAILED(hr))
	{
		log::error << L"Unable to create render view; CreateRenderTargetView failed, HRESULT " << int32_t(hr) << Endl;
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
	dtd.Usage = D3D11_USAGE_DEFAULT;
	dtd.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	dtd.CPUAccessFlags = 0;
	dtd.MiscFlags = 0;

	hr = m_context->getD3DDevice()->CreateTexture2D(&dtd, NULL, &m_d3dDepthStencil.getAssign());
	if (FAILED(hr))
	{
		log::error << L"Unable to create render view; CreateTexture2D failed, HRESULT " << int32_t(hr) << Endl;
		return false;
	}

	std::memset(&ddsvd, 0, sizeof(ddsvd));
	ddsvd.Format = dtd.Format;
	ddsvd.Flags = 0;
	ddsvd.ViewDimension = dtd.SampleDesc.Count > 1 ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;
	ddsvd.Texture2D.MipSlice = 0;

	hr = m_context->getD3DDevice()->CreateDepthStencilView(m_d3dDepthStencil, &ddsvd, &m_d3dDepthStencilView.getAssign());
	if (FAILED(hr))
	{
		log::error << L"Unable to create render view; CreateDepthStencilView failed, HRESULT " << int32_t(hr) << Endl;
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

	m_window->setTitle(!desc.title.empty() ? desc.title.c_str() : L"Traktor - DirectX 11 Renderer");
	m_window->addListener(this);

	m_profiler.create(m_context->getD3DDevice());
	return true;
}

bool RenderViewDx11::reset(int32_t width, int32_t height)
{
	ComRef< ID3D11Texture2D > d3dBackBuffer;
	DXGI_SWAP_CHAIN_DESC scd;
	D3D11_TEXTURE2D_DESC dtd;
	D3D11_DEPTH_STENCIL_VIEW_DESC ddsvd;
	HRESULT hr;

	if (m_window || width <= 0 || height <= 0)
		return false;

	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_context->getLock());
	m_eventQueue.clear();

	m_context->getD3DDeviceContext()->OMSetRenderTargets(0, NULL, NULL);

	m_d3dRenderTargetView.release();
	m_d3dDepthStencil.release();
	m_d3dDepthStencilView.release();

	hr = m_dxgiSwapChain->ResizeBuffers(1, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
	if (FAILED(hr))
	{
		log::error << L"Unable to reset render view; ResizeBuffers failed, HRESULT " << int32_t(hr) << Endl;
		return false;
	}

	hr = m_dxgiSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&d3dBackBuffer.getAssign());
	if (FAILED(hr))
	{
		log::error << L"Unable to reset render view; GetBuffer failed, HRESULT " << int32_t(hr) << Endl;
		return false;
	}

	hr = m_context->getD3DDevice()->CreateRenderTargetView(d3dBackBuffer, NULL, &m_d3dRenderTargetView.getAssign());
	if (FAILED(hr))
	{
		log::error << L"Unable to reset render view; CreateRenderTargetView failed, HRESULT " << int32_t(hr) << Endl;
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
	dtd.Usage = D3D11_USAGE_DEFAULT;
	dtd.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	dtd.CPUAccessFlags = 0;
	dtd.MiscFlags = 0;

	hr = m_context->getD3DDevice()->CreateTexture2D(&dtd, NULL, &m_d3dDepthStencil.getAssign());
	if (FAILED(hr))
	{
		log::error << L"Unable to reset render view; CreateTexture2D failed, HRESULT " << int32_t(hr) << Endl;
		return false;
	}
	
	std::memset(&ddsvd, 0, sizeof(ddsvd));
	ddsvd.Format = dtd.Format;
	ddsvd.Flags = 0;
	ddsvd.ViewDimension = dtd.SampleDesc.Count > 1 ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;
	ddsvd.Texture2D.MipSlice = 0;

	hr = m_context->getD3DDevice()->CreateDepthStencilView(m_d3dDepthStencil, &ddsvd, &m_d3dDepthStencilView.getAssign());
	if (FAILED(hr))
	{
		log::error << L"Unable to reset render view; CreateDepthStencilView failed, HRESULT " << int32_t(hr) << Endl;
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

	m_profiler.create(m_context->getD3DDevice());
	return true;
}

int RenderViewDx11::getWidth() const
{
	return m_targetSize[0];
}

int RenderViewDx11::getHeight() const
{
	return m_targetSize[1];
}

bool RenderViewDx11::isActive() const
{
	if (m_window)
		return GetForegroundWindow() == *m_window;
	else
		return true;
}

bool RenderViewDx11::isFullScreen() const
{
	return m_fullScreen;
}

void RenderViewDx11::showCursor()
{
	m_cursorVisible = true;
}

void RenderViewDx11::hideCursor()
{
	m_cursorVisible = false;
}

bool RenderViewDx11::isCursorVisible() const
{
	return m_cursorVisible;
}

bool RenderViewDx11::setGamma(float gamma)
{
	ComRef< IDXGIOutput > dxgiOutput;
	DXGI_GAMMA_CONTROL gc;
	HRESULT hr;

	hr = m_dxgiSwapChain->GetContainingOutput(&dxgiOutput.getAssign());
	if (FAILED(hr))
		return false;

	hr = dxgiOutput->GetGammaControl(&gc);
	if (FAILED(hr))
		return false;

	for (int32_t i = 0; i < sizeof_array(gc.GammaCurve); ++i)
	{
		gc.GammaCurve[i].Red =
		gc.GammaCurve[i].Green =
		gc.GammaCurve[i].Blue = std::powf(float(i) / (sizeof_array(gc.GammaCurve) - 1), gamma);
	}

	dxgiOutput->SetGammaControl(&gc);
	return true;
}

void RenderViewDx11::setViewport(const Viewport& viewport)
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
		D3D11_VIEWPORT& d3dViewport = m_renderStateStack.back().d3dViewport;

		d3dViewport.TopLeftX = viewport.left;
		d3dViewport.TopLeftY = viewport.top;
		d3dViewport.Width = viewport.width;
		d3dViewport.Height = viewport.height;
		d3dViewport.MinDepth = viewport.nearZ;
		d3dViewport.MaxDepth = viewport.farZ;

		m_context->getD3DDeviceContext()->RSSetViewports(1, &d3dViewport);
	}
}

Viewport RenderViewDx11::getViewport()
{
	const D3D11_VIEWPORT& d3dViewport = m_renderStateStack.empty() ? m_d3dViewport : m_renderStateStack.back().d3dViewport;

	return Viewport(
		d3dViewport.TopLeftX,
		d3dViewport.TopLeftY,
		d3dViewport.Width,
		d3dViewport.Height,
		d3dViewport.MinDepth,
		d3dViewport.MaxDepth
	);
}

SystemWindow RenderViewDx11::getSystemWindow()
{
	SystemWindow sw;
	sw.hWnd = *m_window;
	return sw;
}

bool RenderViewDx11::begin(EyeType eye)
{
	T_ASSERT (m_renderStateStack.empty());

	if (!m_context)
		return false;

	if (!m_context->getLock().wait(1000))
		return false;

	m_profiler.begin(m_context->getD3DDeviceContext());

	RenderState rs =
	{
		m_d3dViewport,
		0,
		{ 0, 0 },
		{ m_d3dRenderTargetView, 0 },
		m_d3dDepthStencilView,
		{ m_targetSize[0], m_targetSize[1] }
	};

	m_renderStateStack.push_back(rs);

	m_drawCalls = 0;
	m_primitiveCount = 0;
	m_targetsDirty = true;

	return true;
}

bool RenderViewDx11::begin(RenderTargetSet* renderTargetSet)
{
	T_ASSERT (!m_renderStateStack.empty());

	if (!m_context)
		return false;

	RenderTargetSetDx11* rts = checked_type_cast< RenderTargetSetDx11*, false >(renderTargetSet);
	RenderTargetDx11* rt0 = checked_type_cast< RenderTargetDx11*, true >(rts->getColorTexture(0));
	RenderTargetDx11* rt1 = checked_type_cast< RenderTargetDx11*, true >(rts->getColorTexture(1));

	if (rt0 && rt1)
	{
		RenderState rs =
		{
			{ 0, 0, rts->getWidth(), rts->getHeight(), 0.0f, 1.0f },
			rts,
			{ rt0, rt1 },
			{ rt0->getD3D11RenderTargetView(), rt1->getD3D11RenderTargetView() },
			rts->getD3D11DepthTextureView(),
			{ rts->getWidth(), rts->getHeight() }
		};

		if (rts->usingPrimaryDepthStencil())
			rs.d3dDepthStencilView = m_d3dDepthStencilView;

		m_renderStateStack.push_back(rs);
		m_targetsDirty = true;
	}
	else if (rt0)
		return begin(renderTargetSet, 0);
	else
		return false;

	return true;
}

bool RenderViewDx11::begin(RenderTargetSet* renderTargetSet, int renderTarget)
{
	T_ASSERT (!m_renderStateStack.empty());

	if (!m_context)
		return false;

	RenderTargetSetDx11* rts = checked_type_cast< RenderTargetSetDx11* >(renderTargetSet);
	RenderTargetDx11* rt = checked_type_cast< RenderTargetDx11* >(rts->getColorTexture(renderTarget));
	RenderState rs =
	{
		{ 0, 0, rts->getWidth(), rts->getHeight(), 0.0f, 1.0f },
		rts,
		{ rt, 0 },
		{ rt->getD3D11RenderTargetView(), 0 },
		rts->getD3D11DepthTextureView(),
		{ rts->getWidth(), rts->getHeight() }
	};

	if (rts->usingPrimaryDepthStencil())
		rs.d3dDepthStencilView = m_d3dDepthStencilView;

	m_renderStateStack.push_back(rs);
	m_targetsDirty = true;

	return true;
}

void RenderViewDx11::clear(uint32_t clearMask, const Color4f* colors, float depth, int32_t stencil)
{
	T_ASSERT (!m_renderStateStack.empty());

	const RenderState& rs = m_renderStateStack.back();

	for (int32_t i = 0; i < 2; ++i)
	{
		if (rs.d3dRenderView[i] != 0 && (clearMask & CfColor) == CfColor)
		{
			float T_MATH_ALIGN16 tmp[4];
			colors[i].storeAligned(tmp);
			m_context->getD3DDeviceContext()->ClearRenderTargetView(rs.d3dRenderView[i], tmp);
		}
	}

	if ((clearMask & (CfDepth | CfStencil)) != 0)
	{
		UINT d3dClear = 0;

		if ((clearMask & CfDepth) != 0)
			d3dClear |= D3D11_CLEAR_DEPTH;
		if ((clearMask & CfStencil) != 0)
			d3dClear |= D3D11_CLEAR_STENCIL;

		if (rs.d3dDepthStencilView)
			m_context->getD3DDeviceContext()->ClearDepthStencilView(rs.d3dDepthStencilView, d3dClear, depth, stencil);
	}
}

void RenderViewDx11::draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives)
{
	T_ASSERT (!m_renderStateStack.empty());

	if (m_currentProgram)
		m_currentProgram->unbind(m_context->getD3DDevice(), m_context->getD3DDeviceContext());

	bindTargets();

	const RenderState& rs = m_renderStateStack.back();

	m_currentVertexBuffer = checked_type_cast< VertexBufferDx11* >(vertexBuffer);
	m_currentIndexBuffer = checked_type_cast< IndexBufferDx11* >(indexBuffer);
	m_currentProgram = checked_type_cast< ProgramDx11* >(program);

	// Prepare buffers.
	m_currentVertexBuffer->prepare(m_context->getD3DDeviceContext(), m_stateCache);
	if (m_currentIndexBuffer)
		m_currentIndexBuffer->prepare(m_context->getD3DDeviceContext(), m_stateCache);

	// Bind program with device, handle input mapping of vertex elements.
	if (!m_currentProgram->bind(
		m_context->getD3DDevice(),
		m_context->getD3DDeviceContext(),
		m_stateCache,
		m_currentVertexBuffer->getD3D11InputElementsHash(),
		m_currentVertexBuffer->getD3D11InputElements(),
		rs.targetSize
	))
		return;

	// Draw primitives.
	T_ASSERT (c_d3dTopology[primitives.type] != D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED);
	m_stateCache.setTopology(c_d3dTopology[primitives.type]);

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
		m_context->getD3DDeviceContext()->DrawIndexed(vertexCount, primitives.offset, 0);
	else
		m_context->getD3DDeviceContext()->Draw(vertexCount, primitives.offset);

	m_drawCalls++;
	m_primitiveCount += primitives.count;
}

void RenderViewDx11::draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives, uint32_t instanceCount)
{
	T_ASSERT (!m_renderStateStack.empty());
	T_ASSERT (instanceCount > 0);

	if (m_currentProgram)
		m_currentProgram->unbind(m_context->getD3DDevice(), m_context->getD3DDeviceContext());

	bindTargets();

	const RenderState& rs = m_renderStateStack.back();

	m_currentVertexBuffer = checked_type_cast< VertexBufferDx11* >(vertexBuffer);
	m_currentIndexBuffer = checked_type_cast< IndexBufferDx11* >(indexBuffer);
	m_currentProgram = checked_type_cast< ProgramDx11* >(program);

	// Prepare buffers.
	m_currentVertexBuffer->prepare(m_context->getD3DDeviceContext(), m_stateCache);
	if (m_currentIndexBuffer)
		m_currentIndexBuffer->prepare(m_context->getD3DDeviceContext(), m_stateCache);

	// Bind program with device, handle input mapping of vertex elements.
	if (!m_currentProgram->bind(
		m_context->getD3DDevice(),
		m_context->getD3DDeviceContext(),
		m_stateCache,
		m_currentVertexBuffer->getD3D11InputElementsHash(),
		m_currentVertexBuffer->getD3D11InputElements(),
		rs.targetSize
	))
		return;

	// Draw primitives.
	T_ASSERT (c_d3dTopology[primitives.type] != D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED);
	m_stateCache.setTopology(c_d3dTopology[primitives.type]);

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
		m_context->getD3DDeviceContext()->DrawIndexedInstanced(vertexCount, instanceCount, primitives.offset, 0, 0);
	else
		m_context->getD3DDeviceContext()->DrawInstanced(vertexCount, instanceCount, primitives.offset, 0);

	m_drawCalls++;
	m_primitiveCount += primitives.count * instanceCount;
}

void RenderViewDx11::end()
{
	T_ASSERT (!m_renderStateStack.empty());

	RenderState& rs = m_renderStateStack.back();
	if (rs.renderTargetSet)
		rs.renderTargetSet->setContentValid(true);

	if (!m_targetsDirty)
	{
		if (rs.renderTarget[0])
			rs.renderTarget[0]->unbind();
		if (rs.renderTarget[1])
			rs.renderTarget[1]->unbind();
	}

	m_renderStateStack.pop_back();
	if (!m_renderStateStack.empty())
		m_targetsDirty = true;
	else
		m_targetsDirty = false;
}

void RenderViewDx11::present()
{
	m_profiler.end(m_context->getD3DDeviceContext());

	m_dxgiSwapChain->Present(m_waitVBlank ? 1 : 0, 0);

	m_context->deleteResources();
	m_context->getLock().release();
}

void RenderViewDx11::pushMarker(const char* const marker)
{
	std::wstring wm = marker ? mbstows(marker) : L"Unnamed"; 
	D3DPERF_BeginEvent(D3DCOLOR_RGBA(255, 255, 255, 255), wm.c_str());
}

void RenderViewDx11::popMarker()
{
	D3DPERF_EndEvent();
}

void RenderViewDx11::getStatistics(RenderViewStatistics& outStatistics) const
{
	outStatistics.drawCalls = m_drawCalls;
	outStatistics.primitiveCount = m_primitiveCount;
	outStatistics.duration = m_profiler.get() / 1e6;
}

bool RenderViewDx11::getBackBufferContent(void* buffer) const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_context->getLock());

	ComRef< ID3D11Texture2D > d3dBackBuffer;
	ComRef< ID3D11Texture2D > d3dReadBackTexture;
	HRESULT hr;

	hr = m_dxgiSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&d3dBackBuffer.getAssign());
	if (FAILED(hr))
		return false;

    D3D11_TEXTURE2D_DESC description;
    d3dBackBuffer->GetDesc(&description);
    description.BindFlags = 0;
    description.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
    description.Usage = D3D11_USAGE_STAGING;

	hr = m_context->getD3DDevice()->CreateTexture2D(&description, 0, &d3dReadBackTexture.getAssign());
	if (FAILED(hr))
		return false;

	m_context->getD3DDeviceContext()->CopyResource(d3dReadBackTexture, d3dBackBuffer);

	D3D11_MAPPED_SUBRESOURCE resource;
	hr = m_context->getD3DDeviceContext()->Map(d3dReadBackTexture, 0, D3D11_MAP_READ_WRITE, 0, &resource);
	if (FAILED(hr))
		return false;

	for (uint32_t y = 0; y < description.Height; ++y)
	{
		const uint8_t* sourceRow = static_cast< const uint8_t* >(resource.pData) + resource.RowPitch * y;
		uint8_t* destinationRow = static_cast< uint8_t* >(buffer) + description.Width * sizeof(uint32_t) * y;
		std::memcpy(
			destinationRow,
			sourceRow,
			description.Width * sizeof(uint32_t)
		);
	}

	m_context->getD3DDeviceContext()->Unmap(d3dReadBackTexture, 0);
	return true;
}

void RenderViewDx11::bindTargets()
{
	if (!m_targetsDirty)
		return;

	RenderState& rs = m_renderStateStack.back();

	ID3D11ShaderResourceView* nullViews[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	m_context->getD3DDeviceContext()->VSSetShaderResources(0, sizeof_array(nullViews), (ID3D11ShaderResourceView**)nullViews);
	m_context->getD3DDeviceContext()->PSSetShaderResources(0, sizeof_array(nullViews), (ID3D11ShaderResourceView**)nullViews);

	m_context->getD3DDeviceContext()->OMSetRenderTargets(2, rs.d3dRenderView, rs.d3dDepthStencilView);
	m_context->getD3DDeviceContext()->RSSetViewports(1, &rs.d3dViewport);

	m_stateCache.reset();
	m_targetsDirty = false;
}

bool RenderViewDx11::windowListenerEvent(Window* window, UINT message, WPARAM wParam, LPARAM lParam, LRESULT& outResult)
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

		if (width <= 0 || height <= 0)
			return false;

		DXGI_SWAP_CHAIN_DESC dxscd;
		std::memset(&dxscd, 0, sizeof(dxscd));

		if (m_dxgiSwapChain)
			m_dxgiSwapChain->GetDesc(&dxscd);

		if (m_dxgiSwapChain == 0 || width != dxscd.BufferDesc.Width || height != dxscd.BufferDesc.Height)
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
		if (m_cursorVisible)
			SetCursor(LoadCursor(NULL, IDC_ARROW));
		else
			SetCursor(NULL);
	}
	else
		return false;

	return true;
}

	}
}
