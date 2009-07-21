#include "Render/Dx9/Platform.h"
#include "Render/Dx9/TypesDx9.h"
#include "Render/Dx9/Win32/RenderTargetSetWin32.h"
#include "Render/Dx9/Win32/RenderTargetWin32.h"
#include "Render/Dx9/ContextDx9.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

TextureFormat c_formatPromote[][2] =
{
	{ TfR16F, TfR32F },
	{ TfR32F, TfR16G16B16A16F },
	{ TfR16G16B16A16F, TfR32G32B32A32F }
};

TextureFormat tryPromoteFormat(TextureFormat format)
{
	for (int i = 0; i < 3; ++i)
	{
		if (c_formatPromote[i][0] == format)
			return c_formatPromote[i][1];
	}
	return format;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderTargetSetWin32", RenderTargetSetWin32, RenderTargetSet)

RenderTargetSetWin32::RenderTargetSetWin32(UnmanagedListener* unmanagedListener, ContextDx9* context)
:	Unmanaged(unmanagedListener)
,	m_context(context)
{
	Unmanaged::addToListener();
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
	lostDevice();
	Unmanaged::removeFromListener();
}

int RenderTargetSetWin32::getWidth() const
{
	return m_desc.width;
}

int RenderTargetSetWin32::getHeight() const
{
	return m_desc.height;
}

ITexture* RenderTargetSetWin32::getColorTexture(int index) const
{
	return index < int(m_colorTextures.size()) ? m_colorTextures[index] : 0;
}

HRESULT RenderTargetSetWin32::lostDevice()
{
	for (RefArray< RenderTargetWin32 >::iterator i = m_colorTextures.begin(); i != m_colorTextures.end(); ++i)
		(*i)->destroy();

	m_colorTextures.resize(0);

	m_d3dDevice.release();
	m_d3dTargetDepthStencilTexture.release();
	m_d3dTargetDepthStencilSurface.release();

	return S_OK;
}

HRESULT RenderTargetSetWin32::resetDevice(IDirect3DDevice9* d3dDevice)
{
	if (!(m_d3dDevice = d3dDevice))
		return S_FALSE;

	if (!internalCreate())
		return S_FALSE;

	setContentValid(false);
	return S_OK;
}

LRESULT RenderTargetSetWin32::internalCreate()
{
	ComRef< IDirect3D9 > d3d;
	D3DCAPS9 d3dCaps;
	D3DDISPLAYMODE d3dDisplayMode;
	HRESULT hr;

	hr = m_d3dDevice->GetDirect3D(&d3d.getAssign());
	if (FAILED(hr))
		return hr;

	hr = m_d3dDevice->GetDeviceCaps(&d3dCaps); 
	if (FAILED(hr))
		return hr;

	hr = m_d3dDevice->GetDisplayMode(0, &d3dDisplayMode);
	if (FAILED(hr))
		return hr;

	// Ensure all target formats is supported; try to promote format if
	// available.
	for (int i = 0; i < m_desc.count; ++i)
	{
		for (;;)
		{
			hr = d3d->CheckDeviceFormat(
				d3dCaps.AdapterOrdinal,
				d3dCaps.DeviceType,
				d3dDisplayMode.Format,
				D3DUSAGE_RENDERTARGET,
				D3DRTYPE_SURFACE,
				c_d3dFormat[m_desc.targets[i].format]
			);
			if (SUCCEEDED(hr))
				break;
			else
			{
				TextureFormat promotedFormat = tryPromoteFormat(m_desc.targets[i].format);
				if (promotedFormat == m_desc.targets[i].format)
				{
					log::error << L"Device doesn't support target format " << getTextureFormatName(m_desc.targets[i].format) << L"." << Endl;
					return hr;
				}
				log::error << L"Device doesn't support target format " << getTextureFormatName(m_desc.targets[i].format) << L"; promoting to " << getTextureFormatName(promotedFormat) << L"." << Endl;
				m_desc.targets[i].format = promotedFormat;
			}
		}
	}

	// Ensure multisample works with all target formats.
	if (m_desc.multiSample)
	{
		for (int i = 0; i < m_desc.count; ++i)
		{
			hr = d3d->CheckDeviceMultiSampleType(
				d3dCaps.AdapterOrdinal,
				d3dCaps.DeviceType,
				c_d3dFormat[m_desc.targets[i].format],
				TRUE,
				(D3DMULTISAMPLE_TYPE)m_desc.multiSample,
				NULL
			);
			if (FAILED(hr))
			{
				log::warning << L"Device doesn't support MSAA on target format; MSAA disabled." << Endl;
				m_desc.multiSample = 0;
				break;
			}
		}
	}

	// Create render targets.
	m_colorTextures.resize(m_desc.count);
	for (int i = 0; i < m_desc.count; ++i)
	{
		m_colorTextures[i] = gc_new< RenderTargetWin32 >(m_context);
		if (!m_colorTextures[i]->create(m_d3dDevice, m_desc, m_desc.targets[i]))
			return S_FALSE;
	}

	// Create depth/stencil surface.
	if (m_desc.depthStencil)
	{
		hr = d3d->CheckDeviceFormat(
			d3dCaps.AdapterOrdinal,
			d3dCaps.DeviceType,
			d3dDisplayMode.Format,
			D3DUSAGE_DEPTHSTENCIL,
			D3DRTYPE_SURFACE,
			D3DFMT_D24S8
		);
		if (FAILED(hr))
			return hr;

		if (m_desc.multiSample > 0)
		{
			hr = m_d3dDevice->CreateDepthStencilSurface(
				m_desc.width,
				m_desc.height,
				D3DFMT_D24S8,
				(D3DMULTISAMPLE_TYPE)m_desc.multiSample,
				0,
				TRUE,
				&m_d3dTargetDepthStencilSurface.getAssign(),
				NULL
			);
			if (FAILED(hr))
				return hr;
		}
		else
		{
			hr = m_d3dDevice->CreateTexture(
				m_desc.width,
				m_desc.height,
				1,
				D3DUSAGE_DEPTHSTENCIL,
				D3DFMT_D24S8,
				D3DPOOL_DEFAULT,
				&m_d3dTargetDepthStencilTexture.getAssign(),
				NULL
			);
			if (FAILED(hr))
				return hr;

			hr = m_d3dTargetDepthStencilTexture->GetSurfaceLevel(0, &m_d3dTargetDepthStencilSurface.getAssign());
			if (FAILED(hr))
				return hr;
		}
	}

	return S_OK;
}

	}
}
