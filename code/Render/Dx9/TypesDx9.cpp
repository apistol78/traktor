/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Render/Dx9/TypesDx9.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

const struct DepthStencilFormats
{
	int32_t depthBits;
	int32_t stencilBits;
	D3DFORMAT d3dDepthBufferFormats[6];
}
c_depthStencilFormats []=
{
#if !defined(_XBOX)
	{ 15, 0, { D3DFMT_D16, D3DFMT_D15S1, D3DFMT_D24X8, D3DFMT_D24S8, D3DFMT_D32, D3DFMT_UNKNOWN } },
#else
	{ 15, 0, { D3DFMT_D16, D3DFMT_D24X8, D3DFMT_D24S8, D3DFMT_D32, D3DFMT_UNKNOWN } },
#endif
	{ 16, 0, { D3DFMT_D16, D3DFMT_D24X8, D3DFMT_D24S8, D3DFMT_D32, D3DFMT_UNKNOWN } },
	{ 24, 0, { D3DFMT_D24X8, D3DFMT_D24S8, D3DFMT_D32, D3DFMT_D16, D3DFMT_UNKNOWN } },
#if !defined(_XBOX)
	{ 15, 1, { D3DFMT_D15S1, D3DFMT_D24S8, D3DFMT_D24X4S4, D3DFMT_UNKNOWN } },
	{ 16, 1, { D3DFMT_D24S8, D3DFMT_D24X4S4, D3DFMT_D15S1, D3DFMT_UNKNOWN } },
	{ 24, 1, { D3DFMT_D24S8, D3DFMT_D24X4S4, D3DFMT_D15S1, D3DFMT_UNKNOWN } },
	{ 15, 4, { D3DFMT_D24S8, D3DFMT_D24X4S4, D3DFMT_UNKNOWN } },
	{ 16, 4, { D3DFMT_D24S8, D3DFMT_D24X4S4, D3DFMT_UNKNOWN } },
	{ 24, 4, { D3DFMT_D24S8, D3DFMT_D24X4S4, D3DFMT_UNKNOWN } },
#else
	{ 15, 1, { D3DFMT_D24S8, D3DFMT_UNKNOWN } },
	{ 16, 1, { D3DFMT_D24S8, D3DFMT_UNKNOWN } },
	{ 24, 1, { D3DFMT_D24S8, D3DFMT_UNKNOWN } },
	{ 15, 4, { D3DFMT_D24S8, D3DFMT_UNKNOWN } },
	{ 16, 4, { D3DFMT_D24S8, D3DFMT_UNKNOWN } },
	{ 24, 4, { D3DFMT_D24S8, D3DFMT_UNKNOWN } },
#endif
	{ 15, 8, { D3DFMT_D24S8, D3DFMT_UNKNOWN } },
	{ 16, 8, { D3DFMT_D24S8, D3DFMT_UNKNOWN } },
	{ 24, 8, { D3DFMT_D24S8, D3DFMT_UNKNOWN } }
};

		}

void textureCopy(void* d, const void* s, uint32_t bytes, TextureFormat textureFormat)
{
	uint32_t blockSize = getTextureBlockSize(textureFormat);
	uint32_t blockCount = bytes / blockSize;

	switch (textureFormat)
	{
	case TfR8G8B8A8:	// D3DFMT_A8R8G8B8
		{
			T_ASSERT (blockSize == 4);

			const uint8_t* sp = static_cast< const uint8_t* >(s);
			uint8_t* dp = static_cast< uint8_t* >(d);

			for (uint32_t i = 0; i < blockCount; ++i)
			{
				dp[0] = sp[2];	// B
				dp[1] = sp[1];	// G
				dp[2] = sp[0];	// R
				dp[3] = sp[3];	// A
				dp += 4;
				sp += 4;
			}
		}
		break;

	case TfR16G16B16A16F:	// D3DFMT_A16B16G16R16F
		{
			T_ASSERT (blockSize == 8);

			const uint16_t* sp = static_cast< const uint16_t* >(s);
			uint16_t* dp = static_cast< uint16_t* >(d);

			for (uint32_t i = 0; i < blockCount; ++i)
			{
				dp[0] = sp[0];
				dp[1] = sp[1];
				dp[2] = sp[2];
				dp[3] = sp[3];
				dp += 4;
				sp += 4;
			}
		}
		break;

	case TfR32G32B32A32F:	// D3DFMT_A32B32G32R32F
		{
			T_ASSERT (blockSize == 16);

			const uint32_t* sp = static_cast< const uint32_t* >(s);
			uint32_t* dp = static_cast< uint32_t* >(d);

			for (uint32_t i = 0; i < blockCount; ++i)
			{
				dp[0] = sp[0];
				dp[1] = sp[1];
				dp[2] = sp[2];
				dp[3] = sp[3];
				dp += 4;
				sp += 4;
			}
		}
		break;

	default:
		memcpy(d, s, bytes);
		break;
	}
}

D3DFORMAT determineDepthStencilFormat(IDirect3D9* d3d, uint16_t depthBits, uint16_t stencilBits, D3DFORMAT d3dBackBufferFormat)
{
	HRESULT hr;
	
	for (uint32_t i = 0; i < sizeof_array(c_depthStencilFormats); ++i)
	{
		if (
			c_depthStencilFormats[i].depthBits >= depthBits &&
			c_depthStencilFormats[i].stencilBits >= stencilBits
		)
		{
			// Found satisfying chain of formats; ensure device supports any of them.
			for (uint32_t j = 0; j < sizeof_array(c_depthStencilFormats[i].d3dDepthBufferFormats); ++j)
			{
				if (c_depthStencilFormats[i].d3dDepthBufferFormats[j] == D3DFMT_UNKNOWN)
					break;
			
				hr = d3d->CheckDeviceFormat(
					D3DADAPTER_DEFAULT,
					D3DDEVTYPE_HAL,
					d3dBackBufferFormat,
					D3DUSAGE_DEPTHSTENCIL,
					D3DRTYPE_SURFACE,
					c_depthStencilFormats[i].d3dDepthBufferFormats[j]
				);
				if (SUCCEEDED(hr))
					return c_depthStencilFormats[i].d3dDepthBufferFormats[j];
			}
		}
	}
	
	return D3DFMT_UNKNOWN;
}

D3DMULTISAMPLE_TYPE determineMultiSampleType(IDirect3D9* d3d, D3DFORMAT d3dColorFormat, D3DFORMAT d3dDepthStencilFormat, int32_t samplesCount)
{
	HRESULT hr;

	hr = d3d->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3dColorFormat, FALSE, c_d3dMultiSample[samplesCount], NULL);
	if (SUCCEEDED(hr))
	{
		hr = d3d->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3dDepthStencilFormat, FALSE, c_d3dMultiSample[samplesCount], NULL);
		if (SUCCEEDED(hr))
			return c_d3dMultiSample[samplesCount];
	}

	return D3DMULTISAMPLE_NONE;
}

	}
}
