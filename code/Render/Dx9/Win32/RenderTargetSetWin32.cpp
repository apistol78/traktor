/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Log/Log.h"
#include "Render/Dx9/Platform.h"
#include "Render/Dx9/ResourceManagerDx9.h"
#include "Render/Dx9/TypesDx9.h"
#include "Render/Dx9/Win32/RenderTargetSetWin32.h"
#include "Render/Dx9/Win32/RenderTargetWin32.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

struct
{
	const wchar_t* const name;
	D3DFORMAT d3dFormat;
	D3DFORMAT d3dFormatPromote;
}
c_d3dFormatPromo[] =
{
	{ L"D3DFMT_L8", D3DFMT_L8, D3DFMT_A8R8G8B8 },
	{ L"D3DFMT_R16F", D3DFMT_R16F, D3DFMT_R32F },
	{ L"D3DFMT_G16R16F", D3DFMT_G16R16F, D3DFMT_G32R32F },
	{ L"D3DFMT_A16B16G16R16F", D3DFMT_A16B16G16R16F, D3DFMT_A32B32G32R32F },
	{ L"D3DFMT_R32F", D3DFMT_R32F, D3DFMT_G32R32F },
	{ L"D3DFMT_G32R32F", D3DFMT_G32R32F, D3DFMT_A32B32G32R32F },

	// Unpromotable
	{ L"D3DFMT_A8R8G8B8", D3DFMT_A8R8G8B8, D3DFMT_UNKNOWN },
	{ L"D3DFMT_A32B32G32R32F", D3DFMT_A32B32G32R32F, D3DFMT_UNKNOWN },
	{ L"D3DFMT_DXT1", D3DFMT_DXT1, D3DFMT_UNKNOWN },
	{ L"D3DFMT_DXT2", D3DFMT_DXT2, D3DFMT_UNKNOWN },
	{ L"D3DFMT_DXT3", D3DFMT_DXT3, D3DFMT_UNKNOWN },
	{ L"D3DFMT_DXT4", D3DFMT_DXT4, D3DFMT_UNKNOWN },
	{ L"D3DFMT_DXT5", D3DFMT_DXT5, D3DFMT_UNKNOWN }
};

const wchar_t* const getD3DFormatName(D3DFORMAT d3dFormat)
{
	for (int i = 0; i < sizeof_array(c_d3dFormatPromo); ++i)
	{
		if (c_d3dFormatPromo[i].d3dFormat == d3dFormat)
			return c_d3dFormatPromo[i].name;
	}
	return L"D3DFMT_???";
}

D3DFORMAT getD3DFormatPromoted(D3DFORMAT d3dFormat)
{
	for (int i = 0; i < sizeof_array(c_d3dFormatPromo); ++i)
	{
		if (c_d3dFormatPromo[i].d3dFormat == d3dFormat)
			return c_d3dFormatPromo[i].d3dFormatPromote;
	}
	return D3DFMT_UNKNOWN;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderTargetSetWin32", RenderTargetSetWin32, RenderTargetSet)

RenderTargetSetWin32::RenderTargetSetWin32(ResourceManagerDx9* resourceManager)
:	m_resourceManager(resourceManager)
,	m_contentValid(false)
{
	m_resourceManager->add(this);
}

RenderTargetSetWin32::~RenderTargetSetWin32()
{
	destroy();
}

bool RenderTargetSetWin32::create(IDirect3DDevice9* d3dDevice, const RenderTargetSetCreateDesc& desc)
{
	m_desc = desc;
	m_d3dDevice = d3dDevice;

	return SUCCEEDED(internalCreate());
}

void RenderTargetSetWin32::destroy()
{
	for (RefArray< RenderTargetWin32 >::iterator i = m_colorTextures.begin(); i != m_colorTextures.end(); ++i)
		(*i)->release();

	m_colorTextures.resize(0);

	m_d3dDevice.release();
	m_d3dTargetDepthStencilTexture.release();
	m_d3dTargetDepthStencilSurface.release();

	m_resourceManager->remove(this);
}

int RenderTargetSetWin32::getWidth() const
{
	return m_desc.width;
}

int RenderTargetSetWin32::getHeight() const
{
	return m_desc.height;
}

ISimpleTexture* RenderTargetSetWin32::getColorTexture(int index) const
{
	return index < int(m_colorTextures.size()) ? m_colorTextures[index] : 0;
}

ISimpleTexture* RenderTargetSetWin32::getDepthTexture() const
{
	return 0;
}

void RenderTargetSetWin32::swap(int index1, int index2)
{
	std::swap(m_colorTextures[index1], m_colorTextures[index2]);
}

void RenderTargetSetWin32::discard()
{
	m_contentValid = false;
}

bool RenderTargetSetWin32::isContentValid() const
{
	return m_contentValid;
}

bool RenderTargetSetWin32::read(int index, void* buffer) const
{
	ComRef< IDirect3DSurface9 > d3dMemorySurface;
	D3DLOCKED_RECT rc;
	HRESULT hr;

	IDirect3DSurface9* d3dRenderTargetSurface = m_colorTextures[index]->getD3DColorSurface();
	if (!d3dRenderTargetSurface)
		return false;

	// Ensure all commands has been executed before we read from surface.
	ComRef< IDirect3DQuery9 > d3dQuery;
	m_d3dDevice->CreateQuery(D3DQUERYTYPE_EVENT, &d3dQuery.getAssign());
	if (d3dQuery)
	{
		d3dQuery->Issue(D3DISSUE_END);
		while (d3dQuery->GetData(NULL, 0, D3DGETDATA_FLUSH) == S_FALSE);
		d3dQuery.release();
	}

	// Create off-screen surface.
	hr = m_d3dDevice->CreateOffscreenPlainSurface(
		m_desc.width,
		m_desc.height,
		D3DFMT_A8R8G8B8,
		D3DPOOL_SYSTEMMEM,
		&d3dMemorySurface.getAssign(),
		NULL
	);
	if (FAILED(hr))
		return false;

	// Copy render target into memory surface.
	hr = m_d3dDevice->GetRenderTargetData(d3dRenderTargetSurface, d3dMemorySurface);
	if (FAILED(hr))
		return false;

	// Copy system memory surface into buffer.
	hr = d3dMemorySurface->LockRect(&rc, NULL, 0);
	if (FAILED(hr))
		return false;

	const uint8_t* sourcePtr = static_cast< const uint8_t* >(rc.pBits);
	uint8_t* destinationPtr = static_cast< uint8_t* >(buffer);

	for (int32_t y = 0; y < m_desc.height; ++y)
	{
		std::memcpy(destinationPtr, sourcePtr, m_desc.width * 4);
		destinationPtr += m_desc.width * 4;
		sourcePtr += rc.Pitch;
	}

	d3dMemorySurface->UnlockRect();
	return true;
}

HRESULT RenderTargetSetWin32::lostDevice()
{
	for (RefArray< RenderTargetWin32 >::iterator i = m_colorTextures.begin(); i != m_colorTextures.end(); ++i)
		(*i)->release();

	m_colorTextures.resize(0);

	m_d3dDevice.release();
	m_d3dTargetDepthStencilTexture.release();
	m_d3dTargetDepthStencilSurface.release();

	m_contentValid = false;
	return S_OK;
}

HRESULT RenderTargetSetWin32::resetDevice(IDirect3DDevice9* d3dDevice)
{
	if (!(m_d3dDevice = d3dDevice))
		return S_FALSE;

	HRESULT hr = internalCreate();
	if (FAILED(hr))
		return hr;

	return S_OK;
}

HRESULT RenderTargetSetWin32::internalCreate()
{
	ComRef< IDirect3D9 > d3d;
	D3DCAPS9 d3dCaps;
	D3DDISPLAYMODE d3dDisplayMode;
	HRESULT hr;

	hr = m_d3dDevice->GetDirect3D(&d3d.getAssign());
	if (FAILED(hr))
	{
		log::error << L"Render target create failed; Unable to get D3D object" << Endl;
		return hr;
	}

	hr = m_d3dDevice->GetDeviceCaps(&d3dCaps); 
	if (FAILED(hr))
	{
		log::error << L"Render target create failed; Unable to get device capabilities" << Endl;
		return hr;
	}

	hr = m_d3dDevice->GetDisplayMode(0, &d3dDisplayMode);
	if (FAILED(hr))
	{
		log::error << L"Render target create failed; Unable to get current display mode" << Endl;
		return hr;
	}

	// Ensure all target formats is supported; try to promote format if
	// available.
	T_ASSERT (m_desc.count < RenderTargetSetCreateDesc::MaxTargets);
	D3DFORMAT d3dFormats[RenderTargetSetCreateDesc::MaxTargets];
	for (int i = 0; i < m_desc.count; ++i)
	{
		d3dFormats[i] = c_d3dFormat[m_desc.targets[i].format];

		// Due to crappy NVidia 7300 GT hw/driver combo we cannot
		// use L8 render targets and there are no safe way of
		// determine as the driver happily report it support
		// the L8 format which it doesn't.
		if (d3dFormats[i] == D3DFMT_L8)
			d3dFormats[i] = D3DFMT_A8R8G8B8;

		for (;;)
		{
			hr = d3d->CheckDeviceFormat(
				d3dCaps.AdapterOrdinal,
				d3dCaps.DeviceType,
				d3dDisplayMode.Format,
				D3DUSAGE_RENDERTARGET,
				D3DRTYPE_SURFACE,
				d3dFormats[i]
			);
			if (SUCCEEDED(hr))
				break;
			else
			{
				D3DFORMAT promotedFormat = getD3DFormatPromoted(d3dFormats[i]);
				if (promotedFormat == D3DFMT_UNKNOWN)
				{
					log::error << L"Render target create failed; Device doesn't support target format \"" << getD3DFormatName(d3dFormats[i]) << L"\"" << Endl;
					return hr;
				}

				T_DEBUG(L"Render target create warning; Device doesn't support target format \"" << getD3DFormatName(d3dFormats[i]) << L"\"; trying with \"" << getD3DFormatName(promotedFormat) << L"\"");
				d3dFormats[i] = promotedFormat;
			}
		}
	}

	// Ensure multi-sample works with all target formats.
	if (m_desc.multiSample)
	{
		for (int i = 0; i < m_desc.count; ++i)
		{
			hr = d3d->CheckDeviceMultiSampleType(
				d3dCaps.AdapterOrdinal,
				d3dCaps.DeviceType,
				d3dFormats[i],
				TRUE,
				c_d3dMultiSample[m_desc.multiSample],
				NULL
			);
			if (FAILED(hr))
			{
				log::error << L"Render target create failed; Device doesn't support MSAA on target format \"" << getD3DFormatName(d3dFormats[i]) << L"\"" << Endl;
				return hr;
			}
		}
	}

	// Create render targets.
	m_colorTextures.resize(m_desc.count);
	for (int i = 0; i < m_desc.count; ++i)
	{
		m_colorTextures[i] = new RenderTargetWin32();
		if (!m_colorTextures[i]->create(m_d3dDevice, m_desc, m_desc.targets[i], d3dFormats[i]))
			return S_FALSE;
	}

	// Create depth/stencil surface.
	if (m_desc.createDepthStencil)
	{
		D3DFORMAT d3dDepthStencilFormat = determineDepthStencilFormat(d3d, 16, 8, d3dDisplayMode.Format);
		if (d3dDepthStencilFormat == D3DFMT_UNKNOWN)
		{
			d3dDepthStencilFormat = determineDepthStencilFormat(d3d, 16, 4, d3dDisplayMode.Format);
			if (d3dDepthStencilFormat == D3DFMT_UNKNOWN)
			{
				d3dDepthStencilFormat = determineDepthStencilFormat(d3d, 16, 1, d3dDisplayMode.Format);
				if (d3dDepthStencilFormat == D3DFMT_UNKNOWN)
				{
					d3dDepthStencilFormat = determineDepthStencilFormat(d3d, 16, 0, d3dDisplayMode.Format);
					if (d3dDepthStencilFormat == D3DFMT_UNKNOWN)
					{
						log::error << L"Render target create failed; Unable to find any supported depth/stencil buffer format" << Endl;
						return S_FALSE;
					}
				}
			}
		}

		D3DMULTISAMPLE_TYPE d3dMultiSample = c_d3dMultiSample[m_desc.multiSample];
		hr = m_d3dDevice->CreateDepthStencilSurface(
			m_desc.width,
			m_desc.height,
			d3dDepthStencilFormat,
			d3dMultiSample,
			0,
			TRUE,
			&m_d3dTargetDepthStencilSurface.getAssign(),
			NULL
		);
		if (FAILED(hr))
		{
			log::error << L"Render target create failed; Unable to create depth/stencil surface" << Endl;
			return hr;
		}
	}

	return S_OK;
}

	}
}
