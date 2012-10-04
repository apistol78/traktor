#include "Core/Log/Log.h"
#include "Render/OpenGL/Platform.h"
#include "Render/OpenGL/Std/Extensions.h"
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

#if defined(GL_IMG_texture_compression_pvrtc)
	case TfPVRTC1:
		outPixelSize = 0;
		outComponents = GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG;
		outFormat = GL_RGB;
		outType = GL_UNSIGNED_BYTE;
		break;

	case TfPVRTC2:
		outPixelSize = 0;
		outComponents = GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG;
		outFormat = GL_RGB;
		outType = GL_UNSIGNED_BYTE;
		break;

	case TfPVRTC3:
		outPixelSize = 0;
		outComponents = GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
		outFormat = GL_RGBA;
		outType = GL_UNSIGNED_BYTE;
		break;

	case TfPVRTC4:
		outPixelSize = 0;
		outComponents = GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG;
		outFormat = GL_RGBA;
		outType = GL_UNSIGNED_BYTE;
		break;
#endif

	default:
		T_DEBUG(L"Unsupported texture outFormat used");
		return false;
	}

	return true;
}

bool convertTargetFormat(TextureFormat targetFormat, GLenum& outInternalFormat, GLint& outFormat, GLenum& outType)
{
	switch (targetFormat)
	{
	case TfR8:
	case TfR8G8B8A8:
		outInternalFormat = GL_RGBA;
		outFormat = GL_RGBA;
		outType = GL_UNSIGNED_BYTE;
		return true;

	case TfR16G16B16A16F:
		if (opengl_have_extension(E_GL_ARB_texture_float))
		{
			outInternalFormat = GL_RGBA16F_ARB;
			outFormat = GL_RGBA;
			outType = GL_HALF_FLOAT_ARB;
			return true;
		}
		break;

	case TfR32G32B32A32F:
		if (opengl_have_extension(E_GL_ARB_texture_float))
		{
			outInternalFormat = GL_RGBA32F_ARB;
			outFormat = GL_RGBA;
			outType = GL_FLOAT;
			return true;
		}
		break;

#if !defined(__APPLE__)
	case TfR16F:
		if (opengl_have_extension(E_GL_ARB_texture_float))
		{
			outInternalFormat = GL_RGBA16F_ARB;
			outFormat = GL_RED;
			outType = GL_HALF_FLOAT_ARB;
			return true;
		}
		else if (opengl_have_extension(E_GL_NV_float_buffer))
		{
			outInternalFormat = GL_FLOAT_R16_NV;
			outFormat = GL_RED;
			outType = GL_FLOAT;
			return true;
		}
		else if (opengl_have_extension(E_GL_ATI_texture_float))
		{
			outInternalFormat = GL_LUMINANCE_FLOAT16_ATI;
			outFormat = GL_RED;
			outType = GL_FLOAT;
			return true;
		}
		break;

	case TfR32F:
		if (opengl_have_extension(E_GL_ARB_texture_float))
		{
			outInternalFormat = GL_RGBA32F_ARB;
			outFormat = GL_RED;
			outType = GL_FLOAT;
			return true;
		}
		else if (opengl_have_extension(E_GL_NV_float_buffer))
		{
			outInternalFormat = GL_FLOAT_R32_NV;
			outFormat = GL_RED;
			outType = GL_FLOAT;
			return true;
		}
		else if (opengl_have_extension(E_GL_ATI_texture_float))
		{
			outInternalFormat = GL_LUMINANCE_FLOAT32_ATI;
			outFormat = GL_RED;
			outType = GL_FLOAT;
			return true;
		}
		break;
#else
	case TfR16F:
		if (opengl_have_extension(E_GL_ARB_texture_float))
		{
			outInternalFormat = GL_RGBA16F_ARB;
			outFormat = GL_RED;
			outType = GL_HALF_FLOAT_ARB;
			return true;
		}
		break;

	case TfR32F:
		if (opengl_have_extension(E_GL_ARB_texture_float))
		{
			outInternalFormat = GL_RGBA32F_ARB;
			outFormat = GL_RED;
			outType = GL_FLOAT;
			return true;
		}
		break;
#endif

	default:
		break;
	}

	T_DEBUG(L"Unsupported target format");
	return false;
}

	}
}
