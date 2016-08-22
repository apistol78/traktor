#include "Core/Log/Log.h"
#include "Core/Thread/Acquire.h"
#include "Render/Types.h"
#include "Render/Dx11/Platform.h"
#include "Render/Dx11/RenderTargetDx11.h"
#include "Render/Dx11/ContextDx11.h"
#include "Render/Dx11/TypesDx11.h"
#include "Render/Dx11/Utilities.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderTargetDx11", RenderTargetDx11, ISimpleTexture)

RenderTargetDx11::RenderTargetDx11(ContextDx11* context)
:	m_context(context)
,	m_d3dColorFormat(DXGI_FORMAT_UNKNOWN)
,	m_width(0)
,	m_height(0)
,	m_generateMips(false)
{
}

RenderTargetDx11::~RenderTargetDx11()
{
	destroy();
}

bool RenderTargetDx11::create(const RenderTargetSetCreateDesc& setDesc, const RenderTargetCreateDesc& desc)
{
	D3D11_TEXTURE2D_DESC dtd;
	D3D11_SHADER_RESOURCE_VIEW_DESC dsrvd;
	HRESULT hr;

	const DXGI_FORMAT* dxgiTextureFormats = desc.sRGB ? c_dxgiTextureFormats_sRGB : c_dxgiTextureFormats;
	if (dxgiTextureFormats[desc.format] == DXGI_FORMAT_UNKNOWN)
		return false;

	m_d3dColorFormat = dxgiTextureFormats[desc.format];

	dtd.Width = setDesc.width;
	dtd.Height = setDesc.height;
	dtd.MipLevels = 1;
	dtd.ArraySize = 1;
	dtd.Format = m_d3dColorFormat;
	dtd.SampleDesc.Count = 1;
	dtd.SampleDesc.Quality = 0;
	dtd.Usage = D3D11_USAGE_DEFAULT;
	dtd.BindFlags = D3D11_BIND_RENDER_TARGET;
	dtd.CPUAccessFlags = 0;
	dtd.MiscFlags = 0;

	// \note Using same heuristics as in RenderTargetDepthDx11.cpp to determine format.
	DXGI_FORMAT d3dDepthFormat = DXGI_FORMAT_UNKNOWN;
	if (setDesc.createDepthStencil)
	{
		if (!setDesc.usingDepthStencilAsTexture)
		{
			if (setDesc.ignoreStencil)
				d3dDepthFormat = DXGI_FORMAT_D16_UNORM;
			else
				d3dDepthFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
		}
		else
			d3dDepthFormat = DXGI_FORMAT_R32_TYPELESS;
	}

	if (d3dDepthFormat != DXGI_FORMAT_UNKNOWN)
	{
		if (!setupSampleDesc(m_context->getD3DDevice(), setDesc.multiSample, m_d3dColorFormat, d3dDepthFormat, dtd.SampleDesc))
			return false;
	}
	else
	{
		if (!setupSampleDesc(m_context->getD3DDevice(), setDesc.multiSample, m_d3dColorFormat, dtd.SampleDesc))
			return false;
	}

	if (dtd.SampleDesc.Count <= 1)
	{
		dtd.MipLevels = setDesc.generateMips ? 0 : 1;
		dtd.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		dtd.MiscFlags = setDesc.generateMips ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0;
	}

	// Create render target texture.
	hr = m_context->getD3DDevice()->CreateTexture2D(
		&dtd,
		NULL,
		&m_d3dTexture.getAssign()
	);
	if (FAILED(hr))
	{
		log::error << L"Unable to create render target, failed to create color texture" << Endl;
		return false;
	}

	hr = m_context->getD3DDevice()->CreateRenderTargetView(m_d3dTexture, NULL, &m_d3dRenderTargetView.getAssign());
	if (FAILED(hr))
	{
		log::error << L"Unable to create render target, failed to create render target view" << Endl;
		return false;
	}

	if (dtd.SampleDesc.Count > 1)
	{
		// Create resolved read-back texture.
		dtd.Width = setDesc.width;
		dtd.Height = setDesc.height;
		dtd.MipLevels = setDesc.generateMips ? 0 : 1;
		dtd.ArraySize = 1;
		dtd.Format = dxgiTextureFormats[desc.format];
		dtd.SampleDesc.Count = 1;
		dtd.SampleDesc.Quality = 0;
		dtd.Usage = D3D11_USAGE_DEFAULT;
		dtd.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		dtd.CPUAccessFlags = 0;
		dtd.MiscFlags = setDesc.generateMips ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0;

		hr = m_context->getD3DDevice()->CreateTexture2D(
			&dtd,
			NULL,
			&m_d3dTextureRead.getAssign()
		);
		if (FAILED(hr))
		{
			log::error << L"Unable to create render target, failed to create read-back color texture" << Endl;
			return false;
		}

	}
	else
	{
		// No multisampling; able to read-back target directly without resolve.
		m_d3dTextureRead = m_d3dTexture;
	}

	// Create shader view to read from read-back texture.
	dsrvd.Format = dtd.Format;
	dsrvd.ViewDimension = /*dtd.SampleDesc.Count > 1 ? D3D11_SRV_DIMENSION_TEXTURE2DMS : */D3D11_SRV_DIMENSION_TEXTURE2D;
	dsrvd.Texture2D.MostDetailedMip = 0;
	dsrvd.Texture2D.MipLevels = -1;

	hr = m_context->getD3DDevice()->CreateShaderResourceView(m_d3dTextureRead, &dsrvd, &m_d3dTextureResourceView.getAssign());
	if (FAILED(hr))
	{
		log::error << L"Unable to create render target, failed to create shader resource view" << Endl;
		return false;
	}

	m_width = setDesc.width;
	m_height = setDesc.height;
	m_generateMips = setDesc.generateMips;

	return true;
}

void RenderTargetDx11::destroy()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_context->getLock());
	m_context->releaseComRef(m_d3dTexture);
	m_context->releaseComRef(m_d3dTextureRead);
	m_context->releaseComRef(m_d3dRenderTargetView);
	m_context->releaseComRef(m_d3dTextureResourceView);
}

ITexture* RenderTargetDx11::resolve()
{
	return this;
}

int RenderTargetDx11::getWidth() const
{
	return m_width;
}

int RenderTargetDx11::getHeight() const
{
	return m_height;
}

bool RenderTargetDx11::lock(int level, Lock& lock)
{
	return false;
}

void RenderTargetDx11::unlock(int level)
{
}

void* RenderTargetDx11::getInternalHandle()
{
	return (m_d3dTexture != m_d3dTextureRead) ? m_d3dTextureRead : m_d3dTexture;
}

void RenderTargetDx11::unbind()
{
	if (m_d3dTexture != m_d3dTextureRead)
	{
		m_context->getD3DDeviceContext()->ResolveSubresource(
			m_d3dTextureRead,
			0,
			m_d3dTexture,
			0,
			m_d3dColorFormat
		);
	}
	if (m_generateMips)
	{
		ID3D11ShaderResourceView* nullViews[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
		m_context->getD3DDeviceContext()->VSSetShaderResources(0, sizeof_array(nullViews), (ID3D11ShaderResourceView**)nullViews);
		m_context->getD3DDeviceContext()->PSSetShaderResources(0, sizeof_array(nullViews), (ID3D11ShaderResourceView**)nullViews);
		m_context->getD3DDeviceContext()->OMSetRenderTargets(0, NULL, NULL);
		m_context->getD3DDeviceContext()->GenerateMips(m_d3dTextureResourceView);
	}
}

	}
}
