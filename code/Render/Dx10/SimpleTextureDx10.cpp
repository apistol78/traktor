#include "Render/Dx10/Platform.h"
#include "Render/Dx10/SimpleTextureDx10.h"
#include "Render/Dx10/ContextDx10.h"
#include "Render/Dx10/TypesDx10.h"
#include "Render/Types.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.SimpleTextureDx10", SimpleTextureDx10, SimpleTexture)

SimpleTextureDx10::SimpleTextureDx10(ContextDx10* context)
:	m_context(context)
,	m_width(0)
,	m_height(0)
{
}

SimpleTextureDx10::~SimpleTextureDx10()
{
	destroy();
}

bool SimpleTextureDx10::create(ID3D10Device* d3dDevice, const SimpleTextureCreateDesc& desc)
{
	D3D10_TEXTURE2D_DESC dtd;
	HRESULT hr;

	T_ASSERT (c_dxgiTextureFormats[desc.format] != DXGI_FORMAT_UNKNOWN);
	T_ASSERT (sizeof(TextureInitialData) == sizeof(D3D10_SUBRESOURCE_DATA));

	std::memset(&dtd, 0, sizeof(dtd));
	dtd.Width = desc.width;
	dtd.Height = desc.height;
	dtd.MipLevels = desc.mipCount;
	dtd.ArraySize = 1;
	dtd.Format = c_dxgiTextureFormats[desc.format];
	dtd.SampleDesc.Count = 1;
	dtd.SampleDesc.Quality = 0;
	dtd.Usage = desc.immutable ? D3D10_USAGE_IMMUTABLE : D3D10_USAGE_DYNAMIC;
	dtd.BindFlags = D3D10_BIND_SHADER_RESOURCE;
	dtd.CPUAccessFlags = desc.immutable ? 0 : D3D10_CPU_ACCESS_WRITE;
	dtd.MiscFlags = 0;

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

	m_width = desc.width;
	m_height = desc.height;

	return true;
}

void SimpleTextureDx10::destroy()
{
	if (!m_context)
		return;
	m_context->releaseComRef(m_d3dTexture);
	m_context->releaseComRef(m_d3dTextureResourceView);
}

int SimpleTextureDx10::getWidth() const
{
	return m_width;
}

int SimpleTextureDx10::getHeight() const
{
	return m_height;
}

int SimpleTextureDx10::getDepth() const
{
	return 1;
}

bool SimpleTextureDx10::lock(int level, Lock& lock)
{
	D3D10_MAPPED_TEXTURE2D dm;
	HRESULT hr;

	hr = m_d3dTexture->Map(level, D3D10_MAP_WRITE_DISCARD, 0, &dm);
	if (FAILED(hr))
		return false;

	lock.pitch = int(dm.RowPitch);
	lock.bits = dm.pData;

	return true;
}

void SimpleTextureDx10::unlock(int level)
{
	m_d3dTexture->Unmap(level);
}

ID3D10ShaderResourceView* SimpleTextureDx10::getD3D10TextureResourceView() const
{
	return m_d3dTextureResourceView;
}

	}
}
