#include "Render/Dx10/Platform.h"
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

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderTargetDx10", RenderTargetDx10, ITexture)

RenderTargetDx10::RenderTargetDx10(ContextDx10* context)
:	m_context(context)
,	m_width(0)
,	m_height(0)
{
}

RenderTargetDx10::~RenderTargetDx10()
{
	destroy();
}

bool RenderTargetDx10::create(ID3D10Device* d3dDevice, const RenderTargetSetCreateDesc& setDesc, const RenderTargetCreateDesc& desc)
{
	D3D10_TEXTURE2D_DESC dtd;
	HRESULT hr;

	dtd.Width = setDesc.width;
	dtd.Height = setDesc.height;
	dtd.MipLevels = 1;
	dtd.ArraySize = 1;
	dtd.Format = c_dxgiTextureFormats[desc.format];
	dtd.SampleDesc.Count = 1;
	dtd.SampleDesc.Quality = 0;
	dtd.Usage = D3D10_USAGE_DEFAULT;
	dtd.BindFlags = D3D10_BIND_RENDER_TARGET | D3D10_BIND_SHADER_RESOURCE;
	dtd.CPUAccessFlags = 0;
	dtd.MiscFlags = 0;

	if (!setupSampleDesc(d3dDevice, setDesc.multiSample, c_dxgiTextureFormats[desc.format], DXGI_FORMAT_D16_UNORM, dtd.SampleDesc))
		return false;

	hr = d3dDevice->CreateTexture2D(
		&dtd,
		NULL,
		&m_d3dTexture.getAssign()
	);
	if (FAILED(hr))
	{
		log::error << L"Unable to create render target, failed to create color texture" << Endl;
		return false;
	}

	hr = d3dDevice->CreateRenderTargetView(m_d3dTexture, NULL, &m_d3dRenderTargetView.getAssign());
	if (FAILED(hr))
	{
		log::error << L"Unable to create render target, failed to create render target view" << Endl;
		return false;
	}

	hr = d3dDevice->CreateShaderResourceView(m_d3dTexture, NULL, &m_d3dTextureResourceView.getAssign());
	if (FAILED(hr))
	{
		log::error << L"Unable to create render target, failed to create shader resource view" << Endl;
		return false;
	}

	m_width = setDesc.width;
	m_height = setDesc.height;

	return true;
}

void RenderTargetDx10::destroy()
{
	if (!m_context)
		return;
	m_context->releaseComRef(m_d3dTexture);
	m_context->releaseComRef(m_d3dRenderTargetView);
	m_context->releaseComRef(m_d3dTextureResourceView);
}

int RenderTargetDx10::getWidth() const
{
	return m_width;
}

int RenderTargetDx10::getHeight() const
{
	return m_height;
}

int RenderTargetDx10::getDepth() const
{
	return 1;
}

	}
}
