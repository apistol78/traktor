#include "Core/Log/Log.h"
#include "Render/Types.h"
#include "Render/Dx11/Platform.h"
#include "Render/Dx11/RenderTargetSetDx11.h"
#include "Render/Dx11/RenderTargetDx11.h"
#include "Render/Dx11/ContextDx11.h"
#include "Render/Dx11/TypesDx11.h"
#include "Render/Dx11/Utilities.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderTargetSetDx11", RenderTargetSetDx11, RenderTargetSet)

RenderTargetSetDx11::RenderTargetSetDx11(ContextDx11* context)
:	RenderTargetSet()
,	m_context(context)
,	m_width(0)
,	m_height(0)
,	m_usingPrimaryDepthStencil(false)
{
}

RenderTargetSetDx11::~RenderTargetSetDx11()
{
	destroy();
}

bool RenderTargetSetDx11::create(ID3D11Device* d3dDevice, const RenderTargetSetCreateDesc& desc)
{
	D3D11_TEXTURE2D_DESC dtd;
	D3D11_DEPTH_STENCIL_VIEW_DESC ddsvd;
	HRESULT hr;

	m_colorTextures.resize(desc.count);
	for (int i = 0; i < desc.count; ++i)
	{
		m_colorTextures[i] = new RenderTargetDx11(m_context);
		if (!m_colorTextures[i]->create(d3dDevice, desc, desc.targets[i]))
			return false;
	}

	if (desc.createDepthStencil)
	{
		std::memset(&dtd, 0, sizeof(dtd));
		dtd.Width = desc.width;
		dtd.Height = desc.height;
		dtd.MipLevels = 1;
		dtd.ArraySize = 1;
		if (desc.ignoreStencil)
			dtd.Format = DXGI_FORMAT_D32_FLOAT;
		else
			dtd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		dtd.SampleDesc.Count = 1;
		dtd.SampleDesc.Quality = 0;
		dtd.Usage = D3D11_USAGE_DEFAULT;
		dtd.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		dtd.CPUAccessFlags = 0;
		dtd.MiscFlags = 0;

		if (!setupSampleDesc(d3dDevice, desc.multiSample, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_D16_UNORM, dtd.SampleDesc))
		{
			log::error << L"Unable to create render target; unsupported MSAA" << Endl;
			return false;
		}

		hr = d3dDevice->CreateTexture2D(&dtd, NULL, &m_d3dDepthTexture.getAssign());
		if (FAILED(hr))
		{
			log::error << L"Unable to create render target; failed to create depth texture" << Endl;
			return false;
		}
		
		std::memset(&ddsvd, 0, sizeof(ddsvd));
		ddsvd.Format = dtd.Format;
		ddsvd.Flags = 0;
		ddsvd.ViewDimension = dtd.SampleDesc.Count > 1 ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;
		ddsvd.Texture2D.MipSlice = 0;

		hr = d3dDevice->CreateDepthStencilView(m_d3dDepthTexture, &ddsvd, &m_d3dDepthTextureView.getAssign());
		if (FAILED(hr))
		{
			log::error << L"Unable to create render target; failed to create depth stencil view" << Endl;
			return false;
		}
	}

	m_width = desc.width;
	m_height = desc.height;
	m_usingPrimaryDepthStencil = desc.usingPrimaryDepthStencil;

	return true;
}

void RenderTargetSetDx11::destroy()
{
	for (RefArray< RenderTargetDx11 >::iterator i = m_colorTextures.begin(); i != m_colorTextures.end(); ++i)
	{
		if (*i)
			(*i)->destroy();
	}

	m_colorTextures.resize(0);

	if (m_context)
	{
		m_context->releaseComRef(m_d3dDepthTexture);
		m_context->releaseComRef(m_d3dDepthTextureView);
	}
}

int RenderTargetSetDx11::getWidth() const
{
	return m_width;
}

int RenderTargetSetDx11::getHeight() const
{
	return m_height;
}

ISimpleTexture* RenderTargetSetDx11::getColorTexture(int index) const
{
	return index < int(m_colorTextures.size()) ? m_colorTextures[index] : 0;
}

void RenderTargetSetDx11::swap(int index1, int index2)
{
	std::swap(m_colorTextures[index1], m_colorTextures[index2]);
}

bool RenderTargetSetDx11::read(int index, void* buffer) const
{
	return false;
}

	}
}
