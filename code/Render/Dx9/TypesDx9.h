/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_TypesDx9_H
#define traktor_render_TypesDx9_H

#include "Core/Config.h"
#include "Render/Types.h"
#include "Render/Dx9/Platform.h"

namespace traktor
{
	namespace render
	{

/*! \ingroup DX9 Xbox360 */
//@{

const D3DFORMAT c_d3dFormat[] =
{
	D3DFMT_UNKNOWN,

	D3DFMT_L8,
	D3DFMT_A8R8G8B8,
	D3DFMT_R5G6B5,
	D3DFMT_A1R5G5B5,
	D3DFMT_A4R4G4B4,

	D3DFMT_UNKNOWN,
	D3DFMT_UNKNOWN,
	D3DFMT_UNKNOWN,
	D3DFMT_UNKNOWN,

	D3DFMT_A16B16G16R16F,
	D3DFMT_A32B32G32R32F,
	D3DFMT_G16R16F,
	D3DFMT_G32R32F,
	D3DFMT_R16F,
	D3DFMT_R32F,
	D3DFMT_A16B16G16R16F,	// TfR11G11B10F

	D3DFMT_UNKNOWN,
	D3DFMT_UNKNOWN,
	D3DFMT_UNKNOWN,
	D3DFMT_UNKNOWN,
	D3DFMT_UNKNOWN,
	D3DFMT_UNKNOWN,
	D3DFMT_UNKNOWN,
	D3DFMT_UNKNOWN,
	D3DFMT_UNKNOWN,
	D3DFMT_UNKNOWN,
	D3DFMT_UNKNOWN,
	D3DFMT_UNKNOWN,
	D3DFMT_UNKNOWN,

	D3DFMT_DXT1,
	D3DFMT_DXT2,
	D3DFMT_DXT3,
	D3DFMT_DXT4,
	D3DFMT_DXT5,

	D3DFMT_UNKNOWN,
	D3DFMT_UNKNOWN,
	D3DFMT_UNKNOWN,
	D3DFMT_UNKNOWN,
	D3DFMT_UNKNOWN,

	D3DFMT_UNKNOWN,
	D3DFMT_UNKNOWN,
	D3DFMT_UNKNOWN,
	D3DFMT_UNKNOWN
};

const D3DMULTISAMPLE_TYPE c_d3dMultiSample[] =
{
	D3DMULTISAMPLE_NONE,
	D3DMULTISAMPLE_NONE,
	D3DMULTISAMPLE_2_SAMPLES,
#if !defined(_XBOX) && !defined(T_USE_XDK)
	D3DMULTISAMPLE_3_SAMPLES,
	D3DMULTISAMPLE_4_SAMPLES,
	D3DMULTISAMPLE_5_SAMPLES,
	D3DMULTISAMPLE_6_SAMPLES,
	D3DMULTISAMPLE_7_SAMPLES,
	D3DMULTISAMPLE_8_SAMPLES,
	D3DMULTISAMPLE_9_SAMPLES,
	D3DMULTISAMPLE_10_SAMPLES,
	D3DMULTISAMPLE_11_SAMPLES,
	D3DMULTISAMPLE_12_SAMPLES,
	D3DMULTISAMPLE_13_SAMPLES,
	D3DMULTISAMPLE_14_SAMPLES,
	D3DMULTISAMPLE_15_SAMPLES,
	D3DMULTISAMPLE_16_SAMPLES
#endif
};

struct ProgramScalar
{
	uint16_t registerIndex;
	uint16_t registerCount;
	uint16_t offset;
	uint16_t length;
};

struct ProgramSampler
{
	uint16_t texture;
	uint16_t stage;	//!< Stage index to bind texture into.
};

struct ProgramTexture
{
	uint16_t texture;
	uint16_t sizeIndex;	//!< Constant index for texture size.
};

//@}

/*! \brief Copy source texture into locked destination texture memory.
 *
 * As DX9 doesn't support proper texture formats we need to emulate
 * the behavior of DX10 and OGL and thus need to transform
 * textures as we copy them over.
 */
void textureCopy(void* d, const void* s, uint32_t bytes, TextureFormat textureFormat);

/*! \brief Determine depth/stencil format from description.
 *
 * \return Depth buffer format, D3DFMT_UNKNOWN if none was found.
 */
D3DFORMAT determineDepthStencilFormat(IDirect3D9* d3d, uint16_t depthBits, uint16_t stencilBits, D3DFORMAT d3dBackBufferFormat);

/*! \brief Determine supported multisample type.
 */
D3DMULTISAMPLE_TYPE determineMultiSampleType(IDirect3D9* d3d, D3DFORMAT d3dColorFormat, D3DFORMAT d3dDepthStencilFormat, int32_t samplesCount);

	}
}

#endif	// traktor_render_TypesDx9_H
