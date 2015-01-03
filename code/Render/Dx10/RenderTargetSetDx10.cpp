#include "Render/Dx10/Platform.h"
#include "Render/Dx10/RenderTargetSetDx10.h"
#include "Render/Dx10/RenderTargetDx10.h"
#include "Render/Dx10/ContextDx10.h"
#include "Render/Dx10/TypesDx10.h"
#include "Render/Dx10/Utilities.h"
#include "Render/Types.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderTargetSetDx10", RenderTargetSetDx10, RenderTargetSet)

RenderTargetSetDx10::RenderTargetSetDx10(ContextDx10* context)
:	m_context(context)
,	m_width(0)
,	m_height(0)
,	m_usingPrimaryDepthStencil(false)
{
}

RenderTargetSetDx10::~RenderTargetSetDx10()
{
	destroy();
}

bool RenderTargetSetDx10::create(ID3D10Device* d3dDevice, const RenderTargetSetCreateDesc& desc)
{
	D3D10_TEXTURE2D_DESC dtd;
	D3D10_DEPTH_STENCIL_VIEW_DESC ddsvd;
	HRESULT hr;

	m_colorTextures.resize(desc.count);
	for (int i = 0; i < desc.count; ++i)
	{
		m_colorTextures[i] = new RenderTargetDx10(m_context);
		if (!m_colorTextures[i]->create(d3dDevice, desc, desc.targets[i]))
			return false;
	}

	if (desc.createDepthStencil)
	{
		dtd.Width = desc.width;
		dtd.Height = desc.height;
		dtd.MipLevels = 1;
		dtd.ArraySize = 1;
		if (desc.ignoreStencil)
			dtd.Format = DXGI_FORMAT_D16_UNORM;
		else
			dtd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		dtd.SampleDesc.Count = 1;
		dtd.SampleDesc.Quality = 0;
		dtd.Usage = D3D10_USAGE_DEFAULT;
		dtd.BindFlags = D3D10_BIND_DEPTH_STENCIL;
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
		
		ddsvd.Format = dtd.Format;
		ddsvd.ViewDimension = dtd.SampleDesc.Count > 1 ? D3D10_DSV_DIMENSION_TEXTURE2DMS : D3D10_DSV_DIMENSION_TEXTURE2D;
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

void RenderTargetSetDx10::destroy()
{
	for (RefArray< RenderTargetDx10 >::iterator i = m_colorTextures.begin(); i != m_colorTextures.end(); ++i)
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

int RenderTargetSetDx10::getWidth() const
{
	return m_width;
}

int RenderTargetSetDx10::getHeight() const
{
	return m_height;
}

ISimpleTexture* RenderTargetSetDx10::getColorTexture(int index) const
{
	return index < int(m_colorTextures.size()) ? m_colorTextures[index] : 0;
}

ISimpleTexture* RenderTargetSetDx10::getDepthTexture() const
{
	return 0;
}

void RenderTargetSetDx10::swap(int index1, int index2)
{
	std::swap(m_colorTextures[index1], m_colorTextures[index2]);
}

void RenderTargetSetDx10::discard()
{
}

bool RenderTargetSetDx10::read(int index, void* buffer) const
{
	return false;
}

	}
}
