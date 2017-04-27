/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Log/Log.h"
#include "Core/Thread/Acquire.h"
#include "Render/Types.h"
#include "Render/Dx11/Platform.h"
#include "Render/Dx11/VolumeTextureDx11.h"
#include "Render/Dx11/ContextDx11.h"
#include "Render/Dx11/TypesDx11.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.VolumeTextureDx11", VolumeTextureDx11, IVolumeTexture)

VolumeTextureDx11::VolumeTextureDx11(ContextDx11* context)
:	m_context(context)
{
}

VolumeTextureDx11::~VolumeTextureDx11()
{
	destroy();
}

bool VolumeTextureDx11::create(const VolumeTextureCreateDesc& desc)
{
	D3D11_TEXTURE3D_DESC dtd;
	HRESULT hr;

	T_ASSERT (sizeof(TextureInitialData) == sizeof(D3D11_SUBRESOURCE_DATA));

	const DXGI_FORMAT* dxgiTextureFormats = desc.sRGB ? c_dxgiTextureFormats_sRGB : c_dxgiTextureFormats;
	if (dxgiTextureFormats[desc.format] == DXGI_FORMAT_UNKNOWN)
		return false;

	std::memset(&dtd, 0, sizeof(dtd));
	dtd.Width = desc.width;
	dtd.Height = desc.height;
	dtd.Depth = desc.depth;
	dtd.MipLevels = desc.mipCount;
	dtd.Format = dxgiTextureFormats[desc.format];
	dtd.Usage = desc.immutable ? D3D11_USAGE_IMMUTABLE : D3D11_USAGE_DYNAMIC;
	dtd.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	dtd.CPUAccessFlags = desc.immutable ? 0 : D3D11_CPU_ACCESS_WRITE;
	dtd.MiscFlags = 0;

	hr = m_context->getD3DDevice()->CreateTexture3D(
		&dtd,
		desc.immutable ? reinterpret_cast< const D3D11_SUBRESOURCE_DATA* >(desc.initialData) : NULL,
		&m_d3dTexture.getAssign()
	);
	if (FAILED(hr))
	{
		log::error << L"Unable to create 3D texture, CreateTexture3D failed (hr = " << int32_t(hr) << L")" << Endl;
		return false;
	}

	hr = m_context->getD3DDevice()->CreateShaderResourceView(m_d3dTexture, NULL, &m_d3dTextureResourceView.getAssign());
	if (FAILED(hr))
	{
		log::error << L"Unable to create 3D texture, CreateShaderResourceView failed (hr = " << int32_t(hr) << L")" << Endl;
		return false;
	}

	m_width = desc.width;
	m_height = desc.height;
	m_depth = desc.depth;

	return true;
}

void VolumeTextureDx11::destroy()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_context->getLock());
	m_context->releaseComRef(m_d3dTexture);
	m_context->releaseComRef(m_d3dTextureResourceView);
}

ITexture* VolumeTextureDx11::resolve()
{
	return this;
}

int VolumeTextureDx11::getWidth() const
{
	return m_width;
}

int VolumeTextureDx11::getHeight() const
{
	return m_height;
}

int VolumeTextureDx11::getDepth() const
{
	return m_depth;
}

ID3D11ShaderResourceView* VolumeTextureDx11::getD3D11TextureResourceView() const
{
	return m_d3dTextureResourceView;
}

	}
}
