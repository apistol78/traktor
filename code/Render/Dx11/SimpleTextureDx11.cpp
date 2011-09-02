#include "Core/Log/Log.h"
#include "Render/Types.h"
#include "Render/Dx11/Platform.h"
#include "Render/Dx11/SimpleTextureDx11.h"
#include "Render/Dx11/ContextDx11.h"
#include "Render/Dx11/TypesDx11.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.SimpleTextureDx11", SimpleTextureDx11, ISimpleTexture)

SimpleTextureDx11::SimpleTextureDx11(ContextDx11* context)
:	m_context(context)
,	m_width(0)
,	m_height(0)
{
}

SimpleTextureDx11::~SimpleTextureDx11()
{
	destroy();
}

bool SimpleTextureDx11::create(const SimpleTextureCreateDesc& desc)
{
	D3D11_TEXTURE2D_DESC dtd;
	HRESULT hr;

	T_ASSERT (c_dxgiTextureFormats[desc.format] != DXGI_FORMAT_UNKNOWN);
	T_ASSERT (sizeof(TextureInitialData) == sizeof(D3D11_SUBRESOURCE_DATA));

	std::memset(&dtd, 0, sizeof(dtd));
	dtd.Width = desc.width;
	dtd.Height = desc.height;
	dtd.MipLevels = desc.mipCount;
	dtd.ArraySize = 1;
	dtd.Format = c_dxgiTextureFormats[desc.format];
	dtd.SampleDesc.Count = 1;
	dtd.SampleDesc.Quality = 0;
	dtd.Usage = desc.immutable ? D3D11_USAGE_IMMUTABLE : D3D11_USAGE_DYNAMIC;
	dtd.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	dtd.CPUAccessFlags = desc.immutable ? 0 : D3D11_CPU_ACCESS_WRITE;
	dtd.MiscFlags = 0;

	if (desc.immutable)
	{
		D3D11_SUBRESOURCE_DATA dsrd[16];
		for (int32_t i = 0; i < desc.mipCount; ++i)
		{
			T_ASSERT (desc.initialData[i].data);
			T_ASSERT (desc.initialData[i].pitch > 0);
			dsrd[i].pSysMem = desc.initialData[i].data;
			dsrd[i].SysMemPitch = desc.initialData[i].pitch;
			dsrd[i].SysMemSlicePitch = desc.initialData[i].slicePitch;
		}
		hr = m_context->getD3DDevice()->CreateTexture2D(&dtd, dsrd, &m_d3dTexture.getAssign());
	}
	else
	{
		hr = m_context->getD3DDevice()->CreateTexture2D(&dtd, NULL, &m_d3dTexture.getAssign());
	}
	if (FAILED(hr))
	{
		log::error << L"Unable to create texture, null texture" << Endl;
		return false;
	}

	hr = m_context->getD3DDevice()->CreateShaderResourceView(m_d3dTexture, NULL, &m_d3dTextureResourceView.getAssign());
	if (FAILED(hr))
	{
		log::error << L"Unable to create texture, failed to create shader view" << Endl;
		return false;
	}

	m_width = desc.width;
	m_height = desc.height;

	return true;
}

void SimpleTextureDx11::destroy()
{
	if (!m_context)
		return;

	m_context->releaseComRef(m_d3dTexture);
	m_context->releaseComRef(m_d3dTextureResourceView);
	m_context = 0;
}

ITexture* SimpleTextureDx11::resolve()
{
	return this;
}

int SimpleTextureDx11::getWidth() const
{
	return m_width;
}

int SimpleTextureDx11::getHeight() const
{
	return m_height;
}

bool SimpleTextureDx11::lock(int level, Lock& lock)
{
	D3D11_MAPPED_SUBRESOURCE dm;
	HRESULT hr;

	hr = m_context->getD3DDeviceContext()->Map(m_d3dTexture, level, D3D11_MAP_WRITE_DISCARD, 0, &dm);
	if (FAILED(hr))
		return false;

	lock.pitch = int(dm.RowPitch);
	lock.bits = dm.pData;

	return true;
}

void SimpleTextureDx11::unlock(int level)
{
	m_context->getD3DDeviceContext()->Unmap(m_d3dTexture, level);
}

ID3D11ShaderResourceView* SimpleTextureDx11::getD3D11TextureResourceView() const
{
	return m_d3dTextureResourceView;
}

	}
}
