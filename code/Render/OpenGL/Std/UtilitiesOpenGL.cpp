#include "Render/OpenGL/Platform.h"
#include "Render/OpenGL/Std/UtilitiesOpenGL.h"

namespace traktor
{
	namespace render
	{

bool convertTextureFormat(TextureFormat textureFormat, int& outPixelSize, GLint& outComponents, GLenum& outFormat, GLenum& outType)
{
	switch (textureFormat)
	{
	case TfR8:
		outPixelSize = 1;
		outComponents = 1;
		outFormat = GL_RED;
		outType = GL_UNSIGNED_BYTE;
		break;

	case TfR8G8B8A8:
		outPixelSize = 4;
		outComponents = GL_RGBA8;
		outFormat = GL_RGBA;
		outType = GL_UNSIGNED_INT_8_8_8_8_REV;
		break;

	//case TfR16G16B16A16F:
	//	break;

	case TfR32G32B32A32F:
		outPixelSize = 16;
		outComponents = 4;
		outFormat = GL_RGBA;
		outType = GL_FLOAT;
		break;

	//case TfR16G16F:
	//	break;

	//case TfR32G32F:
	//	break;

	case TfR16F:
		outPixelSize = 2;
		outComponents = 1;
		outFormat = GL_RED;
		outType = GL_HALF_FLOAT_ARB;
		break;

	case TfR32F:
		outPixelSize = 4;
		outComponents = 1;
		outFormat = GL_RED;
		outType = GL_FLOAT;
		break;

	case TfDXT1:
		outPixelSize = 0;
		outComponents = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
		outFormat = GL_RGBA;
		outType = GL_UNSIGNED_BYTE;
		break;

	case TfDXT2:
	case TfDXT3:
		outPixelSize = 0;
		outComponents = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
		outFormat = GL_RGBA;
		outType = GL_UNSIGNED_BYTE;
		break;

	case TfDXT4:
	case TfDXT5:
		outPixelSize = 0;
		outComponents = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		outFormat = GL_RGBA;
		outType = GL_UNSIGNED_BYTE;
		break;

	default:
		return false;
	}

	return true;
}

	}
}
