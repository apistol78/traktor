#include "Render/Dx9/Platform.h"
#include "Render/Dx9/TypesDx9.h"
#include "Render/Dx9/VolumeTextureDx9.h"
#include "Render/Dx9/ContextDx9.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.VolumeTextureDx9", VolumeTextureDx9, IVolumeTexture)

VolumeTextureDx9::VolumeTextureDx9(ContextDx9* context)
:	m_context(context)
,	m_format(TfInvalid)
,	m_width(0)
,	m_height(0)
,	m_depth(0)
{
}

VolumeTextureDx9::~VolumeTextureDx9()
{
	destroy();
}

bool VolumeTextureDx9::create(IDirect3DDevice9* d3dDevice, const VolumeTextureCreateDesc& desc)
{
	D3DLOCKED_BOX lb;
	HRESULT hr;

	hr = d3dDevice->CreateVolumeTexture(
		desc.width,
		desc.height,
		desc.depth,
		desc.mipCount,
		0,
		c_d3dFormat[desc.format],
		D3DPOOL_MANAGED,
		&m_d3dVolumeTexture.getAssign(),
		NULL
	);
	if (FAILED(hr))
		return false;

	if (desc.immutable)
	{
		for (int i = 0; i < desc.mipCount; ++i)
		{
			if (FAILED(m_d3dVolumeTexture->LockBox(i, &lb, NULL, 0)))
				return false;

			for (int depth = 0; depth < desc.depth; ++depth)
			{
				const unsigned char* s = static_cast< const unsigned char* >(desc.initialData[i].data) + depth * desc.initialData[i].slicePitch;
				unsigned char* d = static_cast< unsigned char* >(lb.pBits) + depth * lb.SlicePitch;

				for (int j = 0; j < (desc.height >> i); ++j)
				{
					textureCopy(d, s, lb.RowPitch, desc.format);
					s += desc.initialData[i].pitch;
					d += lb.RowPitch;
				}
			}

			m_d3dVolumeTexture->UnlockBox(i);
		}
	}

	m_format = desc.format;
	m_width = desc.width;
	m_height = desc.height;
	m_depth = desc.depth;

	return true;
}

void VolumeTextureDx9::destroy()
{
#if !defined(_XBOX) && !defined(T_USE_XDK)
	if (m_context)
		m_context->releaseComRef(m_d3dVolumeTexture);
#endif
}

int VolumeTextureDx9::getWidth() const
{
	return m_width;
}

int VolumeTextureDx9::getHeight() const
{
	return m_height;
}

int VolumeTextureDx9::getDepth() const
{
	return m_depth;
}

IDirect3DBaseTexture9* VolumeTextureDx9::getD3DBaseTexture() const
{
	T_ASSERT (m_d3dVolumeTexture);
	return static_cast< IDirect3DBaseTexture9* >(m_d3dVolumeTexture);
}

	}
}
