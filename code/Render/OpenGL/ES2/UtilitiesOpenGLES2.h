/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#pragma once

#include "Render/Types.h"
#include "Render/OpenGL/ES2/Platform.h"

namespace traktor
{
	namespace render
	{

///*!
// * \ingroup OGL
// */
//bool convertTextureFormat(TextureFormat textureFormat, int& outPixelSize, GLint& outComponents, GLenum& outFormat, GLenum& outType);
//
///*!
// * \ingroup OGL
// */
//bool convertTextureFormat_sRGB(TextureFormat textureFormat, int& outPixelSize, GLint& outComponents, GLenum& outFormat, GLenum& outType);

/*!
 * \ingroup OGL
 */
bool convertTargetFormat(TextureFormat targetFormat, GLenum& outInternalFormat, GLint& outFormat, GLenum& outType);

	}
}
