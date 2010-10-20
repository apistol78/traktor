#include "Render/Dx10/RenderViewDx10.h"
#include "Render/Dx10/ContextDx10.h"
#include "Render/Dx10/VertexBufferDx10.h"
#include "Render/Dx10/IndexBufferDx10.h"
#include "Render/Dx10/ProgramDx10.h"
#include "Render/Dx10/RenderTargetSetDx10.h"
#include "Render/Dx10/RenderTargetDx10.h"
#include "Core/Log/Log.h"

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

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderViewDx10", RenderViewDx10, IRenderView)

RenderViewDx10::RenderViewDx10(
	ContextDx10* context,
	ID3D10Device* d3dDevice,
	IDXGISwapChain* d3dSwapChain,
	const DXGI_SWAP_CHAIN_DESC& scd,
	bool waitVBlank
)
:	m_context(context)
,	m_d3dDevice(d3dDevice)
,	m_d3dSwapChain(d3dSwapChain)
,	m_waitVBlank(waitVBlank)
,	m_dirty(false)
,	m_currentVertexBuffer(0)
,	m_currentIndexBuffer(0)
,	m_currentProgram(0)
{
	ComRef< ID3D10Texture2D > d3dBackBuffer;
	D3D10_TEXTURE2D_DESC dtd;
	D3D10_DEPTH_STENCIL_VIEW_DESC ddsvd;
	HRESULT hr;

	hr = m_d3dSwapChain->GetBuffer(0, __uuidof(ID3D10Texture2D), (void**)&d3dBackBuffer.getAssign());
	T_ASSERT (SUCCEEDED(hr));

	hr = d3dDevice->CreateRenderTargetView(d3dBackBuffer, NULL, &m_d3dRenderTargetView.getAssign());
	T_ASSERT (SUCCEEDED(hr));

	d3dBackBuffer->GetDesc(&dtd);

	dtd.MipLevels = 1;
	dtd.ArraySize = 1;
	dtd.Format = DXGI_FORMAT_D16_UNORM;
	dtd.Usage = D3D10_USAGE_DEFAULT;
	dtd.BindFlags = D3D10_BIND_DEPTH_STENCIL;
	dtd.CPUAccessFlags = 0;
	dtd.MiscFlags = 0;

	hr = d3dDevice->CreateTexture2D(&dtd, NULL, &m_d3dDepthStencil.getAssign());
	T_ASSERT (SUCCEEDED(hr));
	
	ddsvd.Format = dtd.Format;
	ddsvd.ViewDimension = dtd.SampleDesc.Count > 1 ? D3D10_DSV_DIMENSION_TEXTURE2DMS : D3D10_DSV_DIMENSION_TEXTURE2D;
	ddsvd.Texture2D.MipSlice = 0;

	hr = d3dDevice->CreateDepthStencilView(m_d3dDepthStencil, &ddsvd, &m_d3dDepthStencilView.getAssign());
	T_ASSERT (SUCCEEDED(hr));

	m_d3dViewport.TopLeftX = 0;
	m_d3dViewport.TopLeftY = 0;
	m_d3dViewport.Width = scd.BufferDesc.Width;
	m_d3dViewport.Height = scd.BufferDesc.Height;
	m_d3dViewport.MinDepth = 0.0f;
	m_d3dViewport.MaxDepth = 1.0f;
}

RenderViewDx10::~RenderViewDx10()
{
	close();
}

void RenderViewDx10::close()
{
	m_d3dRenderTargetView.release();

	if (m_d3dSwapChain)
	{
		m_d3dSwapChain->SetFullscreenState(FALSE, NULL);
		m_d3dSwapChain.release();
	}
}

bool RenderViewDx10::reset(const RenderViewDefaultDesc& desc)
{
	return false;
}

void RenderViewDx10::resize(int32_t width, int32_t height)
{
	ComRef< ID3D10Texture2D > d3dBackBuffer;
	D3D10_TEXTURE2D_DESC dtd;
	D3D10_DEPTH_STENCIL_VIEW_DESC ddsvd;
	HRESULT hr;

	if (width <= 0 || height <= 0)
		return;

	m_d3dDevice->OMSetRenderTargets(0, NULL, NULL);
	m_d3dRenderTargetView.release();
	m_d3dDepthStencil.release();
	m_d3dDepthStencilView.release();

	hr = m_d3dSwapChain->ResizeBuffers(1, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
	T_ASSERT (SUCCEEDED(hr));

	hr = m_d3dSwapChain->GetBuffer(0, __uuidof(ID3D10Texture2D), (void**)&d3dBackBuffer.getAssign());
	T_ASSERT (SUCCEEDED(hr));

	hr = m_d3dDevice->CreateRenderTargetView(d3dBackBuffer, NULL, &m_d3dRenderTargetView.getAssign());
	T_ASSERT (SUCCEEDED(hr));

	d3dBackBuffer->GetDesc(&dtd);

	dtd.MipLevels = 1;
	dtd.ArraySize = 1;
	dtd.Format = DXGI_FORMAT_D16_UNORM;
	dtd.Usage = D3D10_USAGE_DEFAULT;
	dtd.BindFlags = D3D10_BIND_DEPTH_STENCIL;
	dtd.CPUAccessFlags = 0;
	dtd.MiscFlags = 0;

	hr = m_d3dDevice->CreateTexture2D(&dtd, NULL, &m_d3dDepthStencil.getAssign());
	T_ASSERT (SUCCEEDED(hr));
	
	ddsvd.Format = dtd.Format;
	ddsvd.ViewDimension = dtd.SampleDesc.Count > 1 ? D3D10_DSV_DIMENSION_TEXTURE2DMS : D3D10_DSV_DIMENSION_TEXTURE2D;
	ddsvd.Texture2D.MipSlice = 0;

	hr = m_d3dDevice->CreateDepthStencilView(m_d3dDepthStencil, &ddsvd, &m_d3dDepthStencilView.getAssign());
	T_ASSERT (SUCCEEDED(hr));

	m_d3dViewport.TopLeftX = 0;
	m_d3dViewport.TopLeftY = 0;
	m_d3dViewport.Width = width;
	m_d3dViewport.Height = height;
	m_d3dViewport.MinDepth = 0.0f;
	m_d3dViewport.MaxDepth = 1.0f;
}

int RenderViewDx10::getWidth() const
{
	DXGI_SWAP_CHAIN_DESC desc;
	m_d3dSwapChain->GetDesc(&desc);
	return int(desc.BufferDesc.Width);
}

int RenderViewDx10::getHeight() const
{
	DXGI_SWAP_CHAIN_DESC desc;
	m_d3dSwapChain->GetDesc(&desc);
	return int(desc.BufferDesc.Height);
}

bool RenderViewDx10::isActive() const
{
	return true;
}

bool RenderViewDx10::isFullScreen() const
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

		m_d3dDevice->RSSetViewports(1, &d3dViewport);
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

bool RenderViewDx10::begin(EyeType eye)
{
	T_ASSERT (m_renderStateStack.empty());

	if (!m_d3dDevice)
		return false;

	RenderState rs =
	{
		m_d3dViewport,
		m_d3dRenderTargetView,
		m_d3dDepthStencilView
	};

	m_renderStateStack.push_back(rs);

	m_d3dDevice->OMSetRenderTargets(1, &rs.d3dRenderView, rs.d3dDepthStencilView);
	m_d3dDevice->RSSetViewports(1, &rs.d3dViewport);

	return true;
}

bool RenderViewDx10::begin(RenderTargetSet* renderTargetSet, int renderTarget, bool keepDepthStencil)
{
	T_ASSERT (!m_renderStateStack.empty());

	if (!m_d3dDevice)
		return false;

	RenderTargetSetDx10* rts = checked_type_cast< RenderTargetSetDx10* >(renderTargetSet);
	RenderTargetDx10* rt = checked_type_cast< RenderTargetDx10* >(rts->getColorTexture(renderTarget));
	RenderState rs =
	{
		{ 0, 0, rts->getWidth(), rts->getHeight(), 0.0f, 1.0f },
		rt->getD3D10RenderTargetView(),
		rts->getD3D10DepthTextureView()
	};

	if (keepDepthStencil)
		rs.d3dDepthStencilView = m_renderStateStack.back().d3dDepthStencilView;

	m_renderStateStack.push_back(rs);

	m_d3dDevice->OMSetRenderTargets(1, &rs.d3dRenderView, rs.d3dDepthStencilView);
	m_d3dDevice->RSSetViewports(1, &rs.d3dViewport);

	return true;
}

void RenderViewDx10::clear(uint32_t clearMask, const float color[4], float depth, int32_t stencil)
{
	T_ASSERT (!m_renderStateStack.empty());

	const RenderState& rs = m_renderStateStack.back();

	if (rs.d3dRenderView && (clearMask & CfColor) == CfColor)
		m_d3dDevice->ClearRenderTargetView(rs.d3dRenderView, color);

	if (rs.d3dDepthStencilView && (clearMask & CfDepth) == CfDepth)
		m_d3dDevice->ClearDepthStencilView(rs.d3dDepthStencilView, D3D10_CLEAR_DEPTH, depth, stencil);
}

void RenderViewDx10::setVertexBuffer(VertexBuffer* vertexBuffer)
{
	if (m_currentVertexBuffer != vertexBuffer)
	{
		m_currentVertexBuffer = checked_type_cast< VertexBufferDx10* >(vertexBuffer);
		m_dirty = true;
	}
}

void RenderViewDx10::setIndexBuffer(IndexBuffer* indexBuffer)
{
	if (m_currentIndexBuffer != indexBuffer)
	{
		m_currentIndexBuffer = checked_type_cast< IndexBufferDx10* >(indexBuffer);
		m_dirty = true;
	}
}

void RenderViewDx10::setProgram(IProgram* program)
{
	if (m_currentProgram != program)
	{
		m_currentProgram = checked_type_cast< ProgramDx10* >(program);
		m_dirty = true;
	}
}

void RenderViewDx10::draw(const Primitives& primitives)
{
	T_ASSERT (!m_renderStateStack.empty());

	// Handle dirty resources.
	if (m_dirty)
	{
		// Bind vertex buffer.
		ID3D10Buffer* d3dBuffer = m_currentVertexBuffer->getD3D10Buffer();
		UINT stride = m_currentVertexBuffer->getD3D10Stride(), offset = 0;
		m_d3dDevice->IASetVertexBuffers(0, 1, &d3dBuffer, &stride, &offset);

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

			m_d3dDevice->IASetIndexBuffer(
				m_currentIndexBuffer->getD3D10Buffer(),
				indexFormat,
				0
			);
		}

		m_dirty = false;
	}

	// Bind program with device, handle input mapping of vertex elements.
	if (!m_currentProgram->bind(
		m_d3dDevice,
		size_t(m_currentVertexBuffer.ptr()),
		m_currentVertexBuffer->getD3D10InputElements()
	))
		return;

	// Draw primitives.
	T_ASSERT (c_d3dTopology[primitives.type] != D3D10_PRIMITIVE_TOPOLOGY_UNDEFINED);
	m_d3dDevice->IASetPrimitiveTopology(c_d3dTopology[primitives.type]);

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
		m_d3dDevice->DrawIndexed(vertexCount, primitives.offset, 0);
	else
		m_d3dDevice->Draw(vertexCount, primitives.offset);
}

void RenderViewDx10::end()
{
	T_ASSERT (!m_renderStateStack.empty());

	m_renderStateStack.pop_back();
	if (!m_renderStateStack.empty())
	{
		const RenderState& rs = m_renderStateStack.back();

		m_d3dDevice->OMSetRenderTargets(1, &rs.d3dRenderView, rs.d3dDepthStencilView);
		m_d3dDevice->RSSetViewports(1, &rs.d3dViewport);
	}
}

void RenderViewDx10::present()
{
	m_d3dSwapChain->Present(m_waitVBlank ? 1 : 0, 0);
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
}

	}
}
