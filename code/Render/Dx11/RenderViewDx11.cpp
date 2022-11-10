/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Core/Thread/Acquire.h"
#include "Render/Dx11/BufferViewDx11.h"
#include "Render/Dx11/ContextDx11.h"
#include "Render/Dx11/CubeTextureDx11.h"
#include "Render/Dx11/ProgramDx11.h"
#include "Render/Dx11/RenderTargetDepthDx11.h"
#include "Render/Dx11/RenderTargetDx11.h"
#include "Render/Dx11/RenderTargetSetDx11.h"
#include "Render/Dx11/RenderViewDx11.h"
#include "Render/Dx11/SimpleTextureDx11.h"
#include "Render/Dx11/Utilities.h"
#include "Render/Dx11/VertexLayoutDx11.h"

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
,	m_waitVBlanks(1)
,	m_cursorVisible(true)
,	m_drawCalls(0)
,	m_primitiveCount(0)
,	m_currentVertexBuffer(nullptr)
,	m_currentIndexBuffer(nullptr)
,	m_currentProgram(nullptr)
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
,	m_waitVBlanks(1)
,	m_cursorVisible(true)
,	m_drawCalls(0)
,	m_primitiveCount(0)
,	m_currentVertexBuffer(nullptr)
,	m_currentIndexBuffer(nullptr)
,	m_currentProgram(nullptr)
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
		m_window = nullptr;
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
				log::error << L"Unable to create render view; display mode not supported." << Endl;
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
		log::error << L"Unable to create render view; unsupported MSAA." << Endl;
		return false;
	}

	hr = m_context->getDXGIFactory()->CreateSwapChain(
		m_context->getD3DDevice(),
		&scd,
		&m_dxgiSwapChain.getAssign()
	);
	if (FAILED(hr))
	{
		log::error << L"Unable to create render view; CreateSwapChain failed, HRESULT " << int32_t(hr) << L"." << Endl;
		return false;
	}

	m_context->getDXGIFactory()->MakeWindowAssociation(
		*m_window,
		DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_ALT_ENTER
	);

	hr = m_dxgiSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&d3dBackBuffer.getAssign());
	if (FAILED(hr))
	{
		log::error << L"Unable to create render view; GetBuffer failed, HRESULT " << int32_t(hr) << L"." << Endl;
		return false;
	}

	hr = m_context->getD3DDevice()->CreateRenderTargetView(d3dBackBuffer, NULL, &m_d3dRenderTargetView.getAssign());
	if (FAILED(hr))
	{
		log::error << L"Unable to create render view; CreateRenderTargetView failed, HRESULT " << int32_t(hr) << L"." << Endl;
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
		log::error << L"Unable to create render view; CreateTexture2D failed, HRESULT " << int32_t(hr) << L"." << Endl;
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
		log::error << L"Unable to create render view; CreateDepthStencilView failed, HRESULT " << int32_t(hr) << L"." << Endl;
		return false;
	}

	m_targetSize[0] = scd.BufferDesc.Width;
	m_targetSize[1] = scd.BufferDesc.Height;

	m_fullScreen = desc.fullscreen;
	m_waitVBlanks = desc.waitVBlanks;

	m_window->setTitle(!desc.title.empty() ? desc.title.c_str() : L"Traktor - DirectX 11 Renderer");
	m_window->addListener(this);

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
		log::error << L"Unable to reset render view; ResizeBuffers failed, HRESULT " << int32_t(hr) << L"." << Endl;
		return false;
	}

	hr = m_dxgiSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&d3dBackBuffer.getAssign());
	if (FAILED(hr))
	{
		log::error << L"Unable to reset render view; GetBuffer failed, HRESULT " << int32_t(hr) << L"." << Endl;
		return false;
	}

	hr = m_context->getD3DDevice()->CreateRenderTargetView(d3dBackBuffer, NULL, &m_d3dRenderTargetView.getAssign());
	if (FAILED(hr))
	{
		log::error << L"Unable to reset render view; CreateRenderTargetView failed, HRESULT " << int32_t(hr) << L"." << Endl;
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
		log::error << L"Unable to reset render view; CreateTexture2D failed, HRESULT " << int32_t(hr) << L"." << Endl;
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
		log::error << L"Unable to reset render view; CreateDepthStencilView failed, HRESULT " << int32_t(hr) << L"." << Endl;
		return false;
	}

	m_targetSize[0] = width;
	m_targetSize[1] = height;

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

bool RenderViewDx11::isMinimized() const
{
	return bool(IsIconic(*m_window) == TRUE);
}

bool RenderViewDx11::isFullScreen() const
{
	return m_fullScreen;
}

void RenderViewDx11::showCursor()
{
	if (!m_cursorVisible)
	{
		SetCursor(LoadCursor(NULL, IDC_ARROW));
		m_cursorVisible = true;
	}
}

void RenderViewDx11::hideCursor()
{
	if (m_cursorVisible)
	{
		SetCursor(NULL);
		m_cursorVisible = false;
	}
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
	D3D11_VIEWPORT d3dViewport;
	d3dViewport.TopLeftX = viewport.left;
	d3dViewport.TopLeftY = viewport.top;
	d3dViewport.Width = viewport.width;
	d3dViewport.Height = viewport.height;
	d3dViewport.MinDepth = viewport.nearZ;
	d3dViewport.MaxDepth = viewport.farZ;
	m_context->getD3DDeviceContext()->RSSetViewports(1, &d3dViewport);
}

SystemWindow RenderViewDx11::getSystemWindow()
{
	SystemWindow sw;
	sw.hWnd = *m_window;
	return sw;
}

bool RenderViewDx11::beginFrame()
{
	if (!m_context)
		return false;

	if (!m_context->getLock().wait(1000))
		return false;

	m_drawCalls = 0;
	m_primitiveCount = 0;
	return true;
}

void RenderViewDx11::endFrame()
{
	m_context->deleteResources();
	m_context->getLock().release();
}

void RenderViewDx11::present()
{
	m_dxgiSwapChain->Present(m_waitVBlanks, 0);

	// Check if swap chain is still in same mode as window.
	BOOL fullScreen = FALSE;
	if (SUCCEEDED(m_dxgiSwapChain->GetFullscreenState(&fullScreen, 0)))
	{
		if (m_fullScreen != (fullScreen != FALSE))
		{
			if (m_fullScreen)
				log::warning << L"Unexpected transition, DXGI no longer in fullscreen; need to reset render view." << Endl;
			else
				log::warning << L"Unexpected transition, DXGI in fullscreen; need to reset render view." << Endl;

			m_fullScreen = !m_fullScreen;

			// Issue an event in order to reset view back into either fullscreen or windowed mode.
			RenderEvent evt;
			evt.type = fullScreen ? RenderEventType::SetWindowed: RenderEventType::SetFullScreen;
			m_eventQueue.push_back(evt);
		}
	}
}

bool RenderViewDx11::beginPass(const Clear* clear, uint32_t load, uint32_t store)
{
	RenderState& rs = m_renderState;

	rs = RenderState {
		0,
		{ 0, 0 },
		{ m_d3dRenderTargetView, 0 },
		m_d3dDepthStencilView,
		{ m_targetSize[0], m_targetSize[1] }
	};

	if (clear)
	{
		if ((clear->mask & CfColor) == CfColor)
		{
			float T_MATH_ALIGN16 tmp[4];
			if (rs.d3dRenderView[0] != 0)
			{
				clear->colors[0].storeAligned(tmp);
				m_context->getD3DDeviceContext()->ClearRenderTargetView(rs.d3dRenderView[0], tmp);
			}
		}

		if ((clear->mask & (CfDepth | CfStencil)) != 0)
		{
			if (rs.d3dDepthStencilView)
			{
				UINT d3dClear = 0;
				if ((clear->mask & CfDepth) != 0)
					d3dClear |= D3D11_CLEAR_DEPTH;
				if ((clear->mask & CfStencil) != 0)
					d3dClear |= D3D11_CLEAR_STENCIL;
				m_context->getD3DDeviceContext()->ClearDepthStencilView(rs.d3dDepthStencilView, d3dClear, clear->depth, clear->stencil);
			}
		}
	}

	ID3D11ShaderResourceView* nullViews[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	m_context->getD3DDeviceContext()->VSSetShaderResources(0, sizeof_array(nullViews), (ID3D11ShaderResourceView**)nullViews);
	m_context->getD3DDeviceContext()->PSSetShaderResources(0, sizeof_array(nullViews), (ID3D11ShaderResourceView**)nullViews);

	m_context->getD3DDeviceContext()->OMSetRenderTargets(4, rs.d3dRenderView, rs.d3dDepthStencilView);

	D3D11_VIEWPORT d3dViewport;
	d3dViewport.TopLeftX = 0;
	d3dViewport.TopLeftY = 0;
	d3dViewport.Width = m_targetSize[0];
	d3dViewport.Height = m_targetSize[1];
	d3dViewport.MinDepth = 0.0f;
	d3dViewport.MaxDepth = 1.0f;
	m_context->getD3DDeviceContext()->RSSetViewports(1, &d3dViewport);

	return true;
}

bool RenderViewDx11::beginPass(IRenderTargetSet* renderTargetSet, const Clear* clear, uint32_t load, uint32_t store)
{
	if (!m_context)
		return false;

	RenderState& rs = m_renderState;

	RenderTargetSetDx11* rts = checked_type_cast< RenderTargetSetDx11*, false >(renderTargetSet);
	RenderTargetDepthDx11* rtd = checked_type_cast< RenderTargetDepthDx11*, true >(rts->getDepthTexture());
	RenderTargetDx11* rt0 = checked_type_cast< RenderTargetDx11*, true >(rts->getColorTexture(0));
	RenderTargetDx11* rt1 = checked_type_cast< RenderTargetDx11*, true >(rts->getColorTexture(1));
	RenderTargetDx11* rt2 = checked_type_cast< RenderTargetDx11*, true >(rts->getColorTexture(2));
	RenderTargetDx11* rt3 = checked_type_cast< RenderTargetDx11*, true >(rts->getColorTexture(3));

	if (rt0 && rt1 && rt2 && rt3)
	{
		rs = RenderState {
			rts,
			{ rt0, rt1, rt2, rt3 },
			{ rt0->getD3D11RenderTargetView(), rt1->getD3D11RenderTargetView(), rt2->getD3D11RenderTargetView(), rt3->getD3D11RenderTargetView() },
			(rtd != 0) ? rtd->getD3D11DepthTextureView() : nullptr,
			{ rts->getWidth(), rts->getHeight() }
		};

		if (rts->usingPrimaryDepthStencil())
			rs.d3dDepthStencilView = m_d3dDepthStencilView;
	}
	else if (rt0 && rt1 && rt2)
	{
		rs = RenderState {
			rts,
			{ rt0, rt1, rt2, 0 },
			{ rt0->getD3D11RenderTargetView(), rt1->getD3D11RenderTargetView(), rt2->getD3D11RenderTargetView(), 0 },
			(rtd != 0) ? rtd->getD3D11DepthTextureView() : nullptr,
			{ rts->getWidth(), rts->getHeight() }
		};

		if (rts->usingPrimaryDepthStencil())
			rs.d3dDepthStencilView = m_d3dDepthStencilView;
	}
	else if (rt0 && rt1)
	{
		rs = RenderState {
			rts,
			{ rt0, rt1, 0, 0 },
			{ rt0->getD3D11RenderTargetView(), rt1->getD3D11RenderTargetView(), 0, 0 },
			(rtd != 0) ? rtd->getD3D11DepthTextureView() : nullptr,
			{ rts->getWidth(), rts->getHeight() }
		};

		if (rts->usingPrimaryDepthStencil())
			rs.d3dDepthStencilView = m_d3dDepthStencilView;
	}
	else if (rt0)
	{
		rs = RenderState {
			rts,
			{ rt0, 0, 0, 0 },
			{ rt0->getD3D11RenderTargetView(), 0, 0, 0 },
			(rtd != 0) ? rtd->getD3D11DepthTextureView() : nullptr,
			{ rts->getWidth(), rts->getHeight() }
		};

		if (rts->usingPrimaryDepthStencil())
			rs.d3dDepthStencilView = m_d3dDepthStencilView;
	}
	else if (rtd)
	{
		rs = RenderState {
			rts,
			{ 0, 0, 0, 0 },
			{ 0, 0, 0, 0 },
			rtd->getD3D11DepthTextureView(),
			{ rts->getWidth(), rts->getHeight() }
		};

		if (rts->usingPrimaryDepthStencil())
			rs.d3dDepthStencilView = m_d3dDepthStencilView;
	}
	else
		return false;

	if (clear)
	{
		if ((clear->mask & CfColor) == CfColor)
		{
			float T_MATH_ALIGN16 tmp[4];
			if (rs.d3dRenderView[0] != nullptr)
			{
				clear->colors[0].storeAligned(tmp);
				m_context->getD3DDeviceContext()->ClearRenderTargetView(rs.d3dRenderView[0], tmp);
			}
			if (rs.d3dRenderView[1] != nullptr)
			{
				clear->colors[1].storeAligned(tmp);
				m_context->getD3DDeviceContext()->ClearRenderTargetView(rs.d3dRenderView[1], tmp);
			}
			if (rs.d3dRenderView[2] != nullptr)
			{
				clear->colors[2].storeAligned(tmp);
				m_context->getD3DDeviceContext()->ClearRenderTargetView(rs.d3dRenderView[2], tmp);
			}
			if (rs.d3dRenderView[3] != nullptr)
			{
				clear->colors[3].storeAligned(tmp);
				m_context->getD3DDeviceContext()->ClearRenderTargetView(rs.d3dRenderView[3], tmp);
			}
		}

		if ((clear->mask & (CfDepth | CfStencil)) != 0)
		{
			if (rs.d3dDepthStencilView)
			{
				UINT d3dClear = 0;
				if ((clear->mask & CfDepth) != 0)
					d3dClear |= D3D11_CLEAR_DEPTH;
				if ((clear->mask & CfStencil) != 0)
					d3dClear |= D3D11_CLEAR_STENCIL;
				m_context->getD3DDeviceContext()->ClearDepthStencilView(rs.d3dDepthStencilView, d3dClear, clear->depth, clear->stencil);
			}
		}
	}

	ID3D11ShaderResourceView* nullViews[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	m_context->getD3DDeviceContext()->VSSetShaderResources(0, sizeof_array(nullViews), (ID3D11ShaderResourceView**)nullViews);
	m_context->getD3DDeviceContext()->PSSetShaderResources(0, sizeof_array(nullViews), (ID3D11ShaderResourceView**)nullViews);

	m_context->getD3DDeviceContext()->OMSetRenderTargets(4, rs.d3dRenderView, rs.d3dDepthStencilView);

	D3D11_VIEWPORT d3dViewport;
	d3dViewport.TopLeftX = 0;
	d3dViewport.TopLeftY = 0;
	d3dViewport.Width = rts->getWidth();
	d3dViewport.Height = rts->getHeight();
	d3dViewport.MinDepth = 0.0f;
	d3dViewport.MaxDepth = 1.0f;
	m_context->getD3DDeviceContext()->RSSetViewports(1, &d3dViewport);

	return true;
}

bool RenderViewDx11::beginPass(IRenderTargetSet* renderTargetSet, int32_t renderTarget, const Clear* clear, uint32_t load, uint32_t store)
{
	if (!m_context)
		return false;

	RenderState& rs = m_renderState;

	RenderTargetSetDx11* rts = checked_type_cast< RenderTargetSetDx11*, false >(renderTargetSet);
	RenderTargetDepthDx11* rtd = checked_type_cast< RenderTargetDepthDx11*, true >(rts->getDepthTexture());
	RenderTargetDx11* rt = checked_type_cast< RenderTargetDx11*, false >(rts->getColorTexture(renderTarget));

	rs = RenderState {
		rts,
		{ rt, 0, 0, 0 },
		{ rt->getD3D11RenderTargetView(), 0, 0, 0 },
		(rtd != 0) ? rtd->getD3D11DepthTextureView() : 0,
		{ rts->getWidth(), rts->getHeight() }
	};

	if (rts->usingPrimaryDepthStencil())
		rs.d3dDepthStencilView = m_d3dDepthStencilView;

	if (clear)
	{
		if ((clear->mask & CfColor) == CfColor)
		{
			float T_MATH_ALIGN16 tmp[4];
			if (rs.d3dRenderView[0] != 0)
			{
				clear->colors[0].storeAligned(tmp);
				m_context->getD3DDeviceContext()->ClearRenderTargetView(rs.d3dRenderView[0], tmp);
			}
		}

		if ((clear->mask & (CfDepth | CfStencil)) != 0)
		{
			if (rs.d3dDepthStencilView)
			{
				UINT d3dClear = 0;
				if ((clear->mask & CfDepth) != 0)
					d3dClear |= D3D11_CLEAR_DEPTH;
				if ((clear->mask & CfStencil) != 0)
					d3dClear |= D3D11_CLEAR_STENCIL;
				m_context->getD3DDeviceContext()->ClearDepthStencilView(rs.d3dDepthStencilView, d3dClear, clear->depth, clear->stencil);
			}
		}
	}

	ID3D11ShaderResourceView* nullViews[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	m_context->getD3DDeviceContext()->VSSetShaderResources(0, sizeof_array(nullViews), (ID3D11ShaderResourceView**)nullViews);
	m_context->getD3DDeviceContext()->PSSetShaderResources(0, sizeof_array(nullViews), (ID3D11ShaderResourceView**)nullViews);

	m_context->getD3DDeviceContext()->OMSetRenderTargets(4, rs.d3dRenderView, rs.d3dDepthStencilView);

	D3D11_VIEWPORT d3dViewport;
	d3dViewport.TopLeftX = 0;
	d3dViewport.TopLeftY = 0;
	d3dViewport.Width = rts->getWidth();
	d3dViewport.Height = rts->getHeight();
	d3dViewport.MinDepth = 0.0f;
	d3dViewport.MaxDepth = 1.0f;
	m_context->getD3DDeviceContext()->RSSetViewports(1, &d3dViewport);

	return true;
}

void RenderViewDx11::endPass()
{
	RenderState& rs = m_renderState;

	if (rs.renderTargetSet)
		rs.renderTargetSet->setContentValid(true);

	if (rs.renderTarget[0])
		rs.renderTarget[0]->unbind();
	if (rs.renderTarget[1])
		rs.renderTarget[1]->unbind();
}

void RenderViewDx11::draw(const IBufferView* vertexBuffer, const IVertexLayout* vertexLayout, const IBufferView* indexBuffer, IndexType indexType, IProgram* program, const Primitives& primitives, uint32_t instanceCount)
{
	T_ASSERT(instanceCount > 0);

	const RenderState& rs = m_renderState;

	m_currentVertexLayout = checked_type_cast< const VertexLayoutDx11* >(vertexLayout);
	m_currentVertexBuffer = checked_type_cast< const BufferViewDx11* >(vertexBuffer);
	m_currentIndexBuffer = checked_type_cast< const BufferViewDx11* >(indexBuffer);
	m_currentProgram = checked_type_cast< ProgramDx11* >(program);

	// Prepare buffers.
	m_stateCache.setVertexBuffer(m_currentVertexBuffer->getD3D11Buffer(), m_currentVertexLayout->getD3D11Stride());
	if (m_currentIndexBuffer)
	{
		T_ASSERT(indexType != IndexType::Void);
		if (indexType == IndexType::UInt16)
			m_stateCache.setIndexBuffer(m_currentIndexBuffer->getD3D11Buffer(), DXGI_FORMAT_R16_UINT);
		else if (indexType == IndexType::UInt32)
			m_stateCache.setIndexBuffer(m_currentIndexBuffer->getD3D11Buffer(), DXGI_FORMAT_R32_UINT);
		else
			return;
	}

	// Bind program with device, handle input mapping of vertex elements.
	if (!m_currentProgram->bind(
		m_context->getD3DDevice(),
		m_context->getD3DDeviceContext(),
		m_stateCache,
		m_currentVertexLayout->getD3D11InputElementsHash(),
		m_currentVertexLayout->getD3D11InputElements(),
		rs.targetSize
	))
		return;

	// Draw primitives.
	T_ASSERT(c_d3dTopology[(int32_t)primitives.type] != D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED);
	m_stateCache.setTopology(c_d3dTopology[(int32_t)primitives.type]);

	UINT vertexCount = 0;
	switch (primitives.type)
	{
	case PrimitiveType::Points:
		vertexCount = primitives.count;
		break;

	case PrimitiveType::LineStrip:
		T_ASSERT(0);
		break;

	case PrimitiveType::Lines:
		vertexCount = primitives.count * 2;
		break;

	case PrimitiveType::TriangleStrip:
		vertexCount = primitives.count + 2;
		break;

	case PrimitiveType::Triangles:
		vertexCount = primitives.count * 3;
		break;
	}

	if (primitives.indexed)
		m_context->getD3DDeviceContext()->DrawIndexedInstanced(
			vertexCount,
			instanceCount,
			primitives.offset,
			0,
			0
		);
	else
		m_context->getD3DDeviceContext()->DrawInstanced(
			vertexCount,
			instanceCount,
			primitives.offset,
			0
		);

	m_drawCalls++;
	m_primitiveCount += primitives.count * instanceCount;
}

void RenderViewDx11::drawIndirect(const IBufferView* vertexBuffer, const IVertexLayout* vertexLayout, const IBufferView* indexBuffer, IndexType indexType, IProgram* program, PrimitiveType primitiveType, const IBufferView* drawBuffer, uint32_t drawCount)
{
}

void RenderViewDx11::compute(IProgram* program, const int32_t* workSize)
{
}

bool RenderViewDx11::copy(ITexture* destinationTexture, const Region& destinationRegion, ITexture* sourceTexture, const Region& sourceRegion)
{
	if (!destinationTexture || !sourceTexture)
		return false;

	ID3D11Texture2D* d3dSourceTexture = nullptr;
	ID3D11Texture2D* d3dDestinationTexture = nullptr;
	UINT ssr, dsr;

	if (auto sourceRenderTarget = dynamic_type_cast< RenderTargetDx11* >(sourceTexture))
	{
		d3dSourceTexture = sourceRenderTarget->getD3D11Texture2D();
		ssr = D3D11CalcSubresource(sourceRegion.mip, 0, sourceRenderTarget->getMips());
	}
	else if (auto sourceSimpleTexture = dynamic_type_cast< SimpleTextureDx11* >(sourceTexture))
	{
		d3dSourceTexture = sourceSimpleTexture->getD3D11Texture2D();
		ssr = D3D11CalcSubresource(sourceRegion.mip, 0, sourceSimpleTexture->getMips());
	}
	else if (auto sourceCubeTexture = dynamic_type_cast< CubeTextureDx11* >(sourceTexture))
	{
		d3dSourceTexture = sourceCubeTexture->getD3D11Texture2D();
		ssr = D3D11CalcSubresource(sourceRegion.mip, sourceRegion.z, sourceCubeTexture->getMips());
	}

	if (auto destinationRenderTarget = dynamic_type_cast< RenderTargetDx11* >(destinationTexture))
	{
		d3dDestinationTexture = destinationRenderTarget->getD3D11Texture2D();
		dsr = D3D11CalcSubresource(destinationRegion.mip, 0, destinationRenderTarget->getMips());
	}
	else if (auto destinationSimpleTexture = dynamic_type_cast< SimpleTextureDx11* >(destinationTexture))
	{
		d3dDestinationTexture = destinationSimpleTexture->getD3D11Texture2D();
		dsr = D3D11CalcSubresource(destinationRegion.mip, 0, destinationSimpleTexture->getMips());
	}
	else if (auto destinationCubeTexture = dynamic_type_cast< CubeTextureDx11* >(destinationTexture))
	{
		d3dDestinationTexture = destinationCubeTexture->getD3D11Texture2D();
		dsr = D3D11CalcSubresource(destinationRegion.mip, destinationRegion.z, destinationCubeTexture->getMips());
	}

	if (!d3dDestinationTexture || !d3dSourceTexture)
		return false;

	D3D11_BOX sb;
	sb.left = 0;
	sb.right = sourceRegion.width;
	sb.top = 0;
	sb.bottom = sourceRegion.height;
	sb.front = 0;
	sb.back = 1;

	m_context->getD3DDeviceContext()->CopySubresourceRegion(
		d3dDestinationTexture,
		dsr,
		destinationRegion.x,
		destinationRegion.y,
		0,
		d3dSourceTexture,
		ssr,
		&sb
	);

	return true;
}

int32_t RenderViewDx11::beginTimeQuery()
{
	return -1;
}

void RenderViewDx11::endTimeQuery(int32_t query)
{
}

bool RenderViewDx11::getTimeQuery(int32_t query, bool wait, double& outStart, double& outEnd) const
{
	return false;
}

void RenderViewDx11::pushMarker(const char* const marker)
{
#if defined(T_USE_D3DPERF)
	std::wstring wm = marker ? mbstows(marker) : L"Unnamed";
	D3DPERF_BeginEvent(D3DCOLOR_RGBA(255, 255, 255, 255), wm.c_str());
#endif
}

void RenderViewDx11::popMarker()
{
#if defined(T_USE_D3DPERF)
	D3DPERF_EndEvent();
#endif
}

void RenderViewDx11::getStatistics(RenderViewStatistics& outStatistics) const
{
	outStatistics.drawCalls = m_drawCalls;
	outStatistics.primitiveCount = m_primitiveCount;
}

bool RenderViewDx11::windowListenerEvent(Window* window, UINT message, WPARAM wParam, LPARAM lParam, LRESULT& outResult)
{
	if (message == WM_CLOSE)
	{
		RenderEvent evt;
		evt.type = RenderEventType::Close;
		m_eventQueue.push_back(evt);
	}
	else if (message == WM_SIZE)
	{
		// Remove all pending resize events.
		m_eventQueue.remove_if([](const RenderEvent& evt) {
			return evt.type == RenderEventType::Resize;
		});

		// Push new resize event if not matching current size.
		int32_t width = LOWORD(lParam);
		int32_t height = HIWORD(lParam);

		if (width <= 0 || height <= 0)
			return false;

		DXGI_SWAP_CHAIN_DESC dxscd;
		std::memset(&dxscd, 0, sizeof(dxscd));

		if (m_dxgiSwapChain)
			m_dxgiSwapChain->GetDesc(&dxscd);

		if (m_dxgiSwapChain == nullptr || width != dxscd.BufferDesc.Width || height != dxscd.BufferDesc.Height)
		{
			RenderEvent evt;
			evt.type = RenderEventType::Resize;
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
			evt.type = RenderEventType::ToggleFullScreen;
			m_eventQueue.push_back(evt);
		}
	}
	else if (message == WM_KEYDOWN)
	{
		if (wParam == VK_RETURN && (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0)
		{
			RenderEvent evt;
			evt.type = RenderEventType::ToggleFullScreen;
			m_eventQueue.push_back(evt);
		}
	}
	else if (message == WM_SETCURSOR)
	{
		if (!m_cursorVisible)
			SetCursor(NULL);
		else
			return false;
	}
	else
		return false;

	return true;
}

	}
}
