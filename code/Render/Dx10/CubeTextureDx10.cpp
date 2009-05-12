#include "Render/Dx10/Platform.h"
#include "Render/Dx10/CubeTextureDx10.h"
#include "Render/Dx10/ContextDx10.h"
#include "Render/Dx10/TypesDx10.h"
#include "Render/Types.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.CubeTextureDx10", CubeTextureDx10, CubeTexture)

CubeTextureDx10::CubeTextureDx10(ContextDx10* context)
:	m_context(context)
,	m_side(0)
,	m_mipCount(0)
{
}

CubeTextureDx10::~CubeTextureDx10()
{
	destroy();
}

bool CubeTextureDx10::create(ID3D10Device* d3dDevice, const CubeTextureCreateDesc& desc)
{
	D3D10_TEXTURE2D_DESC dtd;
	HRESULT hr;

	T_ASSERT (c_dxgiTextureFormats[desc.format] != DXGI_FORMAT_UNKNOWN);
	T_ASSERT (sizeof(TextureInitialData) == sizeof(D3D10_SUBRESOURCE_DATA));

	std::memset(&dtd, 0, sizeof(dtd));
	dtd.Width = desc.side;
	dtd.Height = desc.side;
	dtd.MipLevels = desc.mipCount;
	dtd.ArraySize = 6;
	dtd.Format = c_dxgiTextureFormats[desc.format];
	dtd.SampleDesc.Count = 1;
	dtd.SampleDesc.Quality = 0;
	dtd.Usage = desc.immutable ? D3D10_USAGE_IMMUTABLE : D3D10_USAGE_DYNAMIC;
	dtd.BindFlags = D3D10_BIND_SHADER_RESOURCE;
	dtd.CPUAccessFlags = desc.immutable ? 0 : D3D10_CPU_ACCESS_WRITE;
	dtd.MiscFlags = D3D10_RESOURCE_MISC_TEXTURECUBE;

	hr = d3dDevice->CreateTexture2D(
		&dtd,
		desc.immutable ? reinterpret_cast< const D3D10_SUBRESOURCE_DATA* >(desc.initialData) : NULL,
		&m_d3dTexture.getAssign()
	);
	if (FAILED(hr))
	{
		log::error << L"Unable to create texture, null texture" << Endl;
		return false;
	}

	hr = d3dDevice->CreateShaderResourceView(m_d3dTexture, NULL, &m_d3dTextureResourceView.getAssign());
	if (FAILED(hr))
	{
		log::error << L"Unable to create texture, failed to create shader view" << Endl;
		return false;
	}

	m_side = desc.side;
	m_mipCount = desc.mipCount;

	return true;
}

void CubeTextureDx10::destroy()
{
	if (!m_context)
		return;
	m_context->releaseComRef(m_d3dTexture);
	m_context->releaseComRef(m_d3dTextureResourceView);
}

int CubeTextureDx10::getWidth() const
{
	return m_side;
}

int CubeTextureDx10::getHeight() const
{
	return m_side;
}

int CubeTextureDx10::getDepth() const
{
	return m_side;
}

bool CubeTextureDx10::lock(int side, int level, Lock& lock)
{
	D3D10_MAPPED_TEXTURE2D dm;
	HRESULT hr;

	hr = m_d3dTexture->Map(D3D10CalcSubresource(level, side, m_mipCount), D3D10_MAP_WRITE_DISCARD, 0, &dm);
	if (FAILED(hr))
		return false;

	lock.pitch = int(dm.RowPitch);
	lock.bits = dm.pData;

	return true;
}

void CubeTextureDx10::unlock(int side, int level)
{
	m_d3dTexture->Unmap(level);
}

ID3D10ShaderResourceView* CubeTextureDx10::getD3D10TextureResourceView() const
{
	return m_d3dTextureResourceView;
}

	}
}
