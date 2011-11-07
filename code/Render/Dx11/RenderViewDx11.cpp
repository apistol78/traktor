#include "Core/Log/Log.h"
#include "Render/Dx11/RenderViewDx11.h"
#include "Render/Dx11/ContextDx11.h"
#include "Render/Dx11/VertexBufferDx11.h"
#include "Render/Dx11/IndexBufferDx11.h"
#include "Render/Dx11/ProgramDx11.h"
#include "Render/Dx11/RenderTargetSetDx11.h"
#include "Render/Dx11/RenderTargetDx11.h"

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
	Window* window,
	IDXGISwapChain* d3dSwapChain,
	const DXGI_SWAP_CHAIN_DESC& scd,
	bool waitVBlank
)
:	m_context(context)
,	m_d3dSwapChain(d3dSwapChain)
,	m_waitVBlank(waitVBlank)
,	m_dirty(false)
,	m_currentVertexBuffer(0)
,	m_currentIndexBuffer(0)
,	m_currentProgram(0)
{
	ComRef< ID3D11Texture2D > d3dBackBuffer;
	D3D11_TEXTURE2D_DESC dtd;
	D3D11_DEPTH_STENCIL_VIEW_DESC ddsvd;
	HRESULT hr;

	hr = m_d3dSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&d3dBackBuffer.getAssign());
	T_ASSERT (SUCCEEDED(hr));

	hr = m_context->getD3DDevice()->CreateRenderTargetView(d3dBackBuffer, NULL, &m_d3dRenderTargetView.getAssign());
	T_ASSERT (SUCCEEDED(hr));

	d3dBackBuffer->GetDesc(&dtd);

	std::memset(&dtd, 0, sizeof(dtd));
	dtd.Width = scd.BufferDesc.Width;
	dtd.Height = scd.BufferDesc.Height;
	dtd.MipLevels = 1;
	dtd.ArraySize = 1;
	dtd.Format = DXGI_FORMAT_D16_UNORM;
	dtd.SampleDesc.Count = scd.SampleDesc.Count;
	dtd.SampleDesc.Quality = scd.SampleDesc.Quality;
	dtd.Usage = D3D11_USAGE_DEFAULT;
	dtd.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	dtd.CPUAccessFlags = 0;
	dtd.MiscFlags = 0;

	hr = m_context->getD3DDevice()->CreateTexture2D(&dtd, NULL, &m_d3dDepthStencil.getAssign());
	T_ASSERT (SUCCEEDED(hr));
	
	std::memset(&ddsvd, 0, sizeof(ddsvd));
	ddsvd.Format = dtd.Format;
	ddsvd.Flags = 0;
	ddsvd.ViewDimension = dtd.SampleDesc.Count > 1 ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;
	ddsvd.Texture2D.MipSlice = 0;

	hr = m_context->getD3DDevice()->CreateDepthStencilView(m_d3dDepthStencil, &ddsvd, &m_d3dDepthStencilView.getAssign());
	T_ASSERT (SUCCEEDED(hr));

	m_d3dViewport.TopLeftX = 0;
	m_d3dViewport.TopLeftY = 0;
	m_d3dViewport.Width = scd.BufferDesc.Width;
	m_d3dViewport.Height = scd.BufferDesc.Height;
	m_d3dViewport.MinDepth = 0.0f;
	m_d3dViewport.MaxDepth = 1.0f;

	m_targetSize[0] = scd.BufferDesc.Width;
	m_targetSize[1] = scd.BufferDesc.Height;
}

RenderViewDx11::~RenderViewDx11()
{
	close();
}

bool RenderViewDx11::nextEvent(RenderEvent& outEvent)
{
	bool going = true;
	MSG msg;

	while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
	{
		int ret = GetMessage(&msg, NULL, 0, 0);
		if (ret <= 0 || msg.message == WM_QUIT)
			going = false;

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	if (!going)
	{
		outEvent.type = ReClose;
		return true;
	}

	return false;
}

void RenderViewDx11::close()
{
	m_d3dRenderTargetView.release();

	if (m_d3dSwapChain)
	{
		m_d3dSwapChain->SetFullscreenState(FALSE, NULL);
		m_d3dSwapChain.release();
	}
}

bool RenderViewDx11::reset(const RenderViewDefaultDesc& desc)
{
	return false;
}

bool RenderViewDx11::reset(int32_t width, int32_t height)
{
	ComRef< ID3D11Texture2D > d3dBackBuffer;
	D3D11_TEXTURE2D_DESC dtd;
	D3D11_DEPTH_STENCIL_VIEW_DESC ddsvd;
	HRESULT hr;

	if (width <= 0 || height <= 0)
		return false;

	m_context->getD3DDeviceContext()->OMSetRenderTargets(0, NULL, NULL);

	m_d3dRenderTargetView.release();
	m_d3dDepthStencil.release();
	m_d3dDepthStencilView.release();

	hr = m_d3dSwapChain->ResizeBuffers(1, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
	T_ASSERT (SUCCEEDED(hr));

	hr = m_d3dSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&d3dBackBuffer.getAssign());
	T_ASSERT (SUCCEEDED(hr));

	hr = m_context->getD3DDevice()->CreateRenderTargetView(d3dBackBuffer, NULL, &m_d3dRenderTargetView.getAssign());
	T_ASSERT (SUCCEEDED(hr));

	d3dBackBuffer->GetDesc(&dtd);

	dtd.MipLevels = 1;
	dtd.ArraySize = 1;
	dtd.Format = DXGI_FORMAT_D16_UNORM;
	dtd.Usage = D3D11_USAGE_DEFAULT;
	dtd.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	dtd.CPUAccessFlags = 0;
	dtd.MiscFlags = 0;

	hr = m_context->getD3DDevice()->CreateTexture2D(&dtd, NULL, &m_d3dDepthStencil.getAssign());
	T_ASSERT (SUCCEEDED(hr));
	
	ddsvd.Format = dtd.Format;
	ddsvd.ViewDimension = dtd.SampleDesc.Count > 1 ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;
	ddsvd.Texture2D.MipSlice = 0;

	hr = m_context->getD3DDevice()->CreateDepthStencilView(m_d3dDepthStencil, &ddsvd, &m_d3dDepthStencilView.getAssign());
	T_ASSERT (SUCCEEDED(hr));

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
	return true;
}

bool RenderViewDx11::isFullScreen() const
{
	return false;
}

bool RenderViewDx11::setGamma(float gamma)
{
	return false;
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

bool RenderViewDx11::begin(EyeType eye)
{
	T_ASSERT (m_renderStateStack.empty());

	if (!m_context)
		return false;

	RenderState rs =
	{
		m_d3dViewport,
		m_d3dRenderTargetView,
		m_d3dDepthStencilView,
		{ m_targetSize[0], m_targetSize[1] }
	};

	m_renderStateStack.push_back(rs);

	m_context->getD3DDeviceContext()->OMSetRenderTargets(1, &rs.d3dRenderView, rs.d3dDepthStencilView);
	m_context->getD3DDeviceContext()->RSSetViewports(1, &rs.d3dViewport);

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
		rt->getD3D11RenderTargetView(),
		rts->getD3D11DepthTextureView(),
		{ rts->getWidth(), rts->getHeight() }
	};

	if (rts->usingPrimaryDepthStencil())
		rs.d3dDepthStencilView = m_d3dDepthStencilView;

	m_renderStateStack.push_back(rs);

	m_context->getD3DDeviceContext()->OMSetRenderTargets(1, &rs.d3dRenderView, rs.d3dDepthStencilView);
	m_context->getD3DDeviceContext()->RSSetViewports(1, &rs.d3dViewport);

	return true;
}

void RenderViewDx11::clear(uint32_t clearMask, const float color[4], float depth, int32_t stencil)
{
	T_ASSERT (!m_renderStateStack.empty());

	const RenderState& rs = m_renderStateStack.back();

	if (rs.d3dRenderView && (clearMask & CfColor) == CfColor)
		m_context->getD3DDeviceContext()->ClearRenderTargetView(rs.d3dRenderView, color);

	if (rs.d3dDepthStencilView && (clearMask & CfDepth) == CfDepth)
		m_context->getD3DDeviceContext()->ClearDepthStencilView(rs.d3dDepthStencilView, D3D11_CLEAR_DEPTH, depth, stencil);
}

void RenderViewDx11::setVertexBuffer(VertexBuffer* vertexBuffer)
{
	if (m_currentVertexBuffer != vertexBuffer)
	{
		m_currentVertexBuffer = checked_type_cast< VertexBufferDx11* >(vertexBuffer);
		m_dirty = true;
	}
}

void RenderViewDx11::setIndexBuffer(IndexBuffer* indexBuffer)
{
	if (m_currentIndexBuffer != indexBuffer)
	{
		m_currentIndexBuffer = checked_type_cast< IndexBufferDx11* >(indexBuffer);
		m_dirty = true;
	}
}

void RenderViewDx11::setProgram(IProgram* program)
{
	if (m_currentProgram != program)
	{
		m_currentProgram = checked_type_cast< ProgramDx11* >(program);
		m_dirty = true;
	}
}

void RenderViewDx11::draw(const Primitives& primitives)
{
	T_ASSERT (!m_renderStateStack.empty());

	const RenderState& rs = m_renderStateStack.back();

	// Handle dirty resources.
	if (m_dirty)
	{
		// Bind vertex buffer.
		ID3D11Buffer* d3dBuffer = m_currentVertexBuffer->getD3D11Buffer();
		UINT stride = m_currentVertexBuffer->getD3D11Stride(), offset = 0;
		m_context->getD3DDeviceContext()->IASetVertexBuffers(0, 1, &d3dBuffer, &stride, &offset);

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

			m_context->getD3DDeviceContext()->IASetIndexBuffer(
				m_currentIndexBuffer->getD3D11Buffer(),
				indexFormat,
				0
			);
		}

		m_dirty = false;
	}

	// Bind program with device, handle input mapping of vertex elements.
	if (!m_currentProgram->bind(
		m_context->getD3DDevice(),
		m_context->getD3DDeviceContext(),
		size_t(m_currentVertexBuffer.ptr()),
		m_currentVertexBuffer->getD3D11InputElements(),
		rs.targetSize
	))
		return;

	// Draw primitives.
	T_ASSERT (c_d3dTopology[primitives.type] != D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED);
	m_context->getD3DDeviceContext()->IASetPrimitiveTopology(c_d3dTopology[primitives.type]);

	UINT vertexCount;
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
}

void RenderViewDx11::end()
{
	T_ASSERT (!m_renderStateStack.empty());

	m_renderStateStack.pop_back();
	if (!m_renderStateStack.empty())
	{
		const RenderState& rs = m_renderStateStack.back();

		m_context->getD3DDeviceContext()->OMSetRenderTargets(1, &rs.d3dRenderView, rs.d3dDepthStencilView);
		m_context->getD3DDeviceContext()->RSSetViewports(1, &rs.d3dViewport);
	}
}

void RenderViewDx11::present()
{
	m_d3dSwapChain->Present(m_waitVBlank ? 1 : 0, 0);
	m_context->deleteResources();
}

void RenderViewDx11::pushMarker(const char* const marker)
{
}

void RenderViewDx11::popMarker()
{
}

void RenderViewDx11::getStatistics(RenderViewStatistics& outStatistics) const
{
}

	}
}
