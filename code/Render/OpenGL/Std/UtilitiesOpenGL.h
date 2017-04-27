/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_UtilitiesOpenGL_H
#define traktor_render_UtilitiesOpenGL_H

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

#endif	// traktor_render_UtilitiesOpenGL_H
