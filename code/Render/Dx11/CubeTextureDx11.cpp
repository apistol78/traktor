#include "Core/Log/Log.h"
#include "Core/Thread/Acquire.h"
#include "Render/ISimpleTexture.h"
#include "Render/Types.h"
#include "Render/Dx11/Platform.h"
#include "Render/Dx11/CubeTextureDx11.h"
#include "Render/Dx11/ContextDx11.h"
#include "Render/Dx11/TypesDx11.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.CubeTextureDx11", CubeTextureDx11, ICubeTexture)

CubeTextureDx11::CubeTextureDx11(ContextDx11* context)
:	m_context(context)
,	m_side(0)
,	m_mipCount(0)
{
}

CubeTextureDx11::~CubeTextureDx11()
{
	destroy();
}

bool CubeTextureDx11::create(const CubeTextureCreateDesc& desc)
{
	D3D11_TEXTURE2D_DESC dtd;
	HRESULT hr;

	T_ASSERT(sizeof(TextureInitialData) == sizeof(D3D11_SUBRESOURCE_DATA));

	const DXGI_FORMAT* dxgiTextureFormats = desc.sRGB ? c_dxgiTextureFormats_sRGB : c_dxgiTextureFormats;
	if (dxgiTextureFormats[desc.format] == DXGI_FORMAT_UNKNOWN)
		return false;

	std::memset(&dtd, 0, sizeof(dtd));
	dtd.Width = desc.side;
	dtd.Height = desc.side;
	dtd.MipLevels = desc.mipCount;
	dtd.ArraySize = 6;
	dtd.Format = dxgiTextureFormats[desc.format];
	dtd.SampleDesc.Count = 1;
	dtd.SampleDesc.Quality = 0;
	dtd.Usage = desc.immutable ? D3D11_USAGE_IMMUTABLE : D3D11_USAGE_DEFAULT;
	dtd.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	dtd.CPUAccessFlags = 0;
	dtd.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	hr = m_context->getD3DDevice()->CreateTexture2D(
		&dtd,
		desc.immutable ? reinterpret_cast< const D3D11_SUBRESOURCE_DATA* >(desc.initialData) : NULL,
		&m_d3dTexture.getAssign()
	);
	if (FAILED(hr))
	{
		log::error << L"Unable to create cube texture. HRESULT = " << int32_t(hr) << Endl;
		return false;
	}

	if (!desc.immutable)
	{
		std::memset(&dtd, 0, sizeof(dtd));
		dtd.Width = desc.side;
		dtd.Height = desc.side;
		dtd.MipLevels = desc.mipCount;
		dtd.ArraySize = 1;
		dtd.Format = dxgiTextureFormats[desc.format];
		dtd.SampleDesc.Count = 1;
		dtd.SampleDesc.Quality = 0;
		dtd.Usage = D3D11_USAGE_STAGING;
		dtd.BindFlags = 0;
		dtd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
		dtd.MiscFlags = 0;

		hr = m_context->getD3DDevice()->CreateTexture2D(&dtd, NULL, &m_d3dTextureStaging.getAssign());
		if (FAILED(hr))
		{
			log::error << L"Unable to create cube staging texture. HRESULT = " << int32_t(hr) << Endl;
			return false;
		}
	}

	hr = m_context->getD3DDevice()->CreateShaderResourceView(m_d3dTexture, NULL, &m_d3dTextureResourceView.getAssign());
	if (FAILED(hr))
	{
		log::error << L"Unable to create cube texture; Failed to create shader view. HRESULT = " << int32_t(hr) << Endl;
		return false;
	}

	m_side = desc.side;
	m_mipCount = desc.mipCount;

	return true;
}

void CubeTextureDx11::destroy()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_context->getLock());
	m_context->releaseComRef(m_d3dTexture);
	m_context->releaseComRef(m_d3dTextureStaging);
	m_context->releaseComRef(m_d3dTextureResourceView);
}

ITexture* CubeTextureDx11::resolve()
{
	return this;
}

int32_t CubeTextureDx11::getSide() const
{
	return m_side;
}

int32_t CubeTextureDx11::getMips() const
{
	return m_mipCount;
}

bool CubeTextureDx11::lock(int32_t side, int32_t level, Lock& lock)
{
	D3D11_MAPPED_SUBRESOURCE dm;
	HRESULT hr;

	hr = m_context->getD3DDeviceContext()->Map(m_d3dTextureStaging, level, D3D11_MAP_WRITE, 0, &dm);
	if (FAILED(hr))
		return false;

	lock.pitch = int(dm.RowPitch);
	lock.bits = dm.pData;

	return true;
}

void CubeTextureDx11::unlock(int32_t side, int32_t level)
{
	m_context->getD3DDeviceContext()->Unmap(m_d3dTextureStaging, level);

	D3D11_BOX sr;
	sr.left = 0;
	sr.right = m_side >> level;
	sr.top = 0;
	sr.bottom = m_side >> level;
	sr.front = 0;
	sr.back = 1;

	m_context->getD3DDeviceContext()->CopySubresourceRegion(
		m_d3dTexture,
		D3D11CalcSubresource(level, side, m_mipCount),
		0,
		0,
		0,
		m_d3dTextureStaging,
		level,
		&sr
	);
}

	}
}
