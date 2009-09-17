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

	}
}

#endif	// traktor_render_UtilitiesOpenGL_H
