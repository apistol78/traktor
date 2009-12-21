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
	D3DFMT_A16B16G16R16F,
	D3DFMT_A32B32G32R32F,
	D3DFMT_G16R16F,
	D3DFMT_G32R32F,
	D3DFMT_R16F,
	D3DFMT_R32F,
	D3DFMT_DXT1,
	D3DFMT_DXT2,
	D3DFMT_DXT3,
	D3DFMT_DXT4,
	D3DFMT_DXT5
};

//@}

/*! \brief Copy source texture into locked destination texture memory.
 *
 * As DX9 doesn't support proper texture formats we need to emulate
 * the behavior of DX10 and OGL and thus need to transform
 * textures as we copy them over.
 */
void textureCopy(void* d, const void* s, uint32_t bytes, TextureFormat textureFormat);

	}
}

#endif	// traktor_render_TypesDx9_H
