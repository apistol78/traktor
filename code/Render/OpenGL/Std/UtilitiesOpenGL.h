#pragma once

#include "Render/Types.h"

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup OGL
 */
bool convertTextureFormat(TextureFormat textureFormat, int& outPixelSize, GLint& outComponents, GLenum& outFormat, GLenum& outType);

/*!
 * \ingroup OGL
 */
bool convertTextureFormat_sRGB(TextureFormat textureFormat, int& outPixelSize, GLint& outComponents, GLenum& outFormat, GLenum& outType);

/*!
 * \ingroup OGL
 */
bool convertTargetFormat(TextureFormat targetFormat, GLenum& outInternalFormat, GLint& outFormat, GLenum& outType);

	}
}

