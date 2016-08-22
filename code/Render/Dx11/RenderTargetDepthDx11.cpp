#include "Core/Log/Log.h"
#include "Core/Thread/Acquire.h"
#include "Render/Types.h"
#include "Render/Dx11/Platform.h"
#include "Render/Dx11/RenderTargetDepthDx11.h"
#include "Render/Dx11/ContextDx11.h"
#include "Render/Dx11/TypesDx11.h"
#include "Render/Dx11/Utilities.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderTargetDepthDx11", RenderTargetDepthDx11, ISimpleTexture)

RenderTargetDepthDx11::RenderTargetDepthDx11(ContextDx11* context)
:	m_context(context)
,	m_width(0)
,	m_height(0)
{
}

RenderTargetDepthDx11::~RenderTargetDepthDx11()
{
	destroy();
}

bool RenderTargetDepthDx11::create(const RenderTargetSetCreateDesc& setDesc)
{
	D3D11_TEXTURE2D_DESC dtd;
	D3D11_DEPTH_STENCIL_VIEW_DESC ddsvd;
	D3D11_SHADER_RESOURCE_VIEW_DESC dsrvd;
	HRESULT hr;

	// Create DX11 depth texture.
	std::memset(&dtd, 0, sizeof(dtd));
	dtd.Width = setDesc.width;
	dtd.Height = setDesc.height;
	dtd.MipLevels = 1;
	dtd.ArraySize = 1;
	dtd.Format = DXGI_FORMAT_UNKNOWN;
	dtd.SampleDesc.Count = 1;
	dtd.SampleDesc.Quality = 0;
	dtd.Usage = D3D11_USAGE_DEFAULT;
	dtd.BindFlags = 0;
	dtd.CPUAccessFlags = 0;
	dtd.MiscFlags = 0;

	if (!setDesc.usingDepthStencilAsTexture)
	{
		if (setDesc.ignoreStencil)
			dtd.Format = DXGI_FORMAT_D16_UNORM;
		else
			dtd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

		dtd.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	}
	else
	{
		if (!setDesc.ignoreStencil)
		{
			log::error << L"Unable to create render target; doesn't support \"read-back\" depth buffers with attached stencil" << Endl;
			return false;
		}

		dtd.Format = DXGI_FORMAT_R32_TYPELESS;
		dtd.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	}

	if (!setupSampleDesc(m_context->getD3DDevice(), setDesc.multiSample, DXGI_FORMAT_R8G8B8A8_UNORM, dtd.Format, dtd.SampleDesc))
	{
		log::error << L"Unable to create render target; unsupported MSAA" << Endl;
		return false;
	}

	hr = m_context->getD3DDevice()->CreateTexture2D(&dtd, NULL, &m_d3dTexture.getAssign());
	if (FAILED(hr))
	{
		log::error << L"Unable to create render target; failed to create depth texture" << Endl;
		return false;
	}
	
	// Create DX11 depth/stencil view.
	std::memset(&ddsvd, 0, sizeof(ddsvd));
	ddsvd.Format = DXGI_FORMAT_UNKNOWN;
	ddsvd.Flags = 0;
	ddsvd.ViewDimension = dtd.SampleDesc.Count > 1 ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;
	ddsvd.Texture2D.MipSlice = 0;

	if (!setDesc.usingDepthStencilAsTexture)
	{
		if (setDesc.ignoreStencil)
			ddsvd.Format = DXGI_FORMAT_D16_UNORM;
		else
			ddsvd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	}
	else
	{
		ddsvd.Format = DXGI_FORMAT_D32_FLOAT;
	}

	hr = m_context->getD3DDevice()->CreateDepthStencilView(m_d3dTexture, &ddsvd, &m_d3dDepthTextureView.getAssign());
	if (FAILED(hr))
	{
		log::error << L"Unable to create render target; failed to create depth stencil view" << Endl;
		return false;
	}

	// Create DX11 shader resource view.
	if (setDesc.usingDepthStencilAsTexture)
	{
		std::memset(&dsrvd, 0, sizeof(dsrvd));
		dsrvd.Format = DXGI_FORMAT_R32_FLOAT;
		dsrvd.ViewDimension = dtd.SampleDesc.Count > 1 ? D3D11_SRV_DIMENSION_TEXTURE2DMS : D3D11_SRV_DIMENSION_TEXTURE2D;
		dsrvd.Texture2D.MostDetailedMip = 0;
		dsrvd.Texture2D.MipLevels = 1;

		hr = m_context->getD3DDevice()->CreateShaderResourceView(m_d3dTexture, &dsrvd, &m_d3dTextureResourceView.getAssign());
		if (FAILED(hr))
		{
			log::error << L"Unable to create render target, failed to create shader resource view of depth buffer" << Endl;
			return false;
		}
	}

	return true;
}

void RenderTargetDepthDx11::destroy()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_context->getLock());
	m_context->releaseComRef(m_d3dTexture);
	m_context->releaseComRef(m_d3dDepthTextureView);
	m_context->releaseComRef(m_d3dTextureResourceView);
}

ITexture* RenderTargetDepthDx11::resolve()
{
	return this;
}

int RenderTargetDepthDx11::getWidth() const
{
	return m_width;
}

int RenderTargetDepthDx11::getHeight() const
{
	return m_height;
}

bool RenderTargetDepthDx11::lock(int level, Lock& lock)
{
	return false;
}

void RenderTargetDepthDx11::unlock(int level)
{
}

void* RenderTargetDepthDx11::getInternalHandle()
{
	return m_d3dTexture;
}

	}
}
