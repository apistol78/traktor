/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Log/Log.h"
#include "Core/Math/Log2.h"
#include "Render/Dx9/Platform.h"
#include "Render/Dx9/TypesDx9.h"
#include "Render/Dx9/ResourceManagerDx9.h"
#include "Render/Dx9/SimpleTextureDx9.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.SimpleTextureDx9", SimpleTextureDx9, ISimpleTexture)

SimpleTextureDx9::SimpleTextureDx9(ResourceManagerDx9* resourceManager)
:	m_resourceManager(resourceManager)
,	m_format(TfInvalid)
,	m_width(0)
,	m_height(0)
,	m_lock(0)
{
	m_resourceManager->add(this);
}

SimpleTextureDx9::~SimpleTextureDx9()
{
	destroy();
}

bool SimpleTextureDx9::create(IDirect3DDevice9* d3dDevice, const SimpleTextureCreateDesc& desc)
{
	D3DLOCKED_RECT rc;
	HRESULT hr;

	D3DFORMAT d3dFormat = c_d3dFormat[desc.format];

#if defined(_XBOX)

	if (!desc.immutable || !isLog2(desc.width) || !isLog2(desc.height))
		d3dFormat = (D3DFORMAT)MAKELINFMT(d3dFormat);

	d3dFormat = (D3DFORMAT)MAKELEFMT(d3dFormat);

#endif

	hr = d3dDevice->CreateTexture(
		desc.width,
		desc.height,
		desc.mipCount,
		0,
		d3dFormat,
		D3DPOOL_MANAGED,
		&m_d3dTexture.getAssign(),
		NULL
	);
	if (FAILED(hr))
		return false;

	if (desc.immutable)
	{
		for (int i = 0; i < desc.mipCount; ++i)
		{
			if (FAILED(m_d3dTexture->LockRect(i, &rc, NULL, 0)))
				return false;

			const uint8_t* s = static_cast< const uint8_t* >(desc.initialData[i].data);
			uint8_t* d = static_cast< uint8_t* >(rc.pBits);

#if defined(_XBOX)
			if (!XGIsTiledFormat(d3dFormat))
			{
#endif

			int blockRows = (desc.height >> i) / getTextureBlockDenom(desc.format);
			if (blockRows < 1)
				blockRows = 1;

			for (int j = 0; j < blockRows; ++j)
			{
#if defined(_XBOX)
				XMemCpyStreaming(d, s, desc.initialData[i].pitch);
#else
				textureCopy(d, s, desc.initialData[i].pitch, desc.format);
#endif
				s += desc.initialData[i].pitch;
				d += rc.Pitch;
			}

#if defined(_XBOX)
			}
			else
			{
				XGTileTextureLevel(
					desc.width,
					desc.height,
					i,
					XGGetGpuFormat(d3dFormat),
					(XGIsBorderTexture(m_d3dTexture) ? XGTILE_BORDER : 0) | (XGIsPackedTexture(m_d3dTexture) ? 0 : XGTILE_NONPACKED),
					rc.pBits,
					NULL,
					s,
					desc.initialData[i].pitch,
					NULL
				);
			}
#endif

			m_d3dTexture->UnlockRect(i);
		}
	}

	m_format = desc.format;
	m_width = desc.width;
	m_height = desc.height;

	return true;
}

void SimpleTextureDx9::destroy()
{
	m_d3dTexture.release();
	m_resourceManager->remove(this);
}

ITexture* SimpleTextureDx9::resolve()
{
	return this;
}

int SimpleTextureDx9::getWidth() const
{
	return m_width;
}

int SimpleTextureDx9::getHeight() const
{
	return m_height;
}

bool SimpleTextureDx9::lock(int level, Lock& lock)
{
	if (m_lock)
		return false;

	uint32_t size = getTextureMipPitch(m_format, m_width, m_height, level);
	m_lock = new uint8_t [size];

	lock.bits = m_lock;
	lock.pitch = getTextureRowPitch(m_format, m_width, level);

	return true;
}

void SimpleTextureDx9::unlock(int level)
{
	if (!m_lock)
		return;

	D3DLOCKED_RECT rc;

	if (SUCCEEDED(m_d3dTexture->LockRect(level, &rc, NULL, 0)))
	{
		uint32_t size = getTextureMipPitch(m_format, m_width, m_height, level);
		textureCopy(rc.pBits, m_lock, size, m_format);

		m_d3dTexture->UnlockRect(level);
	}

	delete[] m_lock;
	m_lock = 0;
}

void* SimpleTextureDx9::getInternalHandle()
{
	return m_d3dTexture;
}

HRESULT SimpleTextureDx9::lostDevice()
{
	return S_OK;
}

HRESULT SimpleTextureDx9::resetDevice(IDirect3DDevice9* d3dDevice)
{
	return S_OK;
}

	}
}
