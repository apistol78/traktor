#include <cstring>
#include "Core/Log/Log.h"
#include "Render/OpenGL/ES/Platform.h"
#include "Render/OpenGL/ES/UtilitiesOpenGLES.h"

namespace traktor
{
	namespace render
	{
		namespace
		{
		
bool haveExtension(const char* extension)
{
	int32_t extensionLength = strlen(extension);
	const char* supported = (const char*)glGetString(GL_EXTENSIONS);
	while (supported && *supported)
	{
		const char* end = supported;
		while (*end && *end != ' ')
		{
			if ((++end - supported) >= 200)
				break;
		}
		
		int32_t length = end - supported;
		if (
			length == extensionLength &&
			strncmp(supported, extension, length) == 0
		)
			return true;
			
		supported = end;
		while (*supported == ' ')
		{
			if ((++supported - end) >= 10)
				break;
		}
	}
	return false;
}

		}

//bool convertTextureFormat(TextureFormat textureFormat, int& outPixelSize, GLint& outComponents, GLenum& outFormat, GLenum& outType)
//{
//	switch (textureFormat)
//	{
//	case TfR8:
//		outPixelSize = 1;
//		outComponents = GL_R8;
//		outFormat = GL_RED;
//		outType = GL_UNSIGNED_BYTE;
//		break;
//
//	case TfR8G8B8A8:
//		outPixelSize = 4;
//		outComponents = GL_RGBA8;
//		outFormat = GL_RGBA;
//		outType = GL_UNSIGNED_INT_8_8_8_8_REV;
//		break;
//
//	case TfR32G32B32A32F:
//		outPixelSize = 16;
//		outComponents = GL_RGBA32F;
//		outFormat = GL_RGBA;
//		outType = GL_FLOAT;
//		break;
//
//	case TfR16F:
//		outPixelSize = 2;
//		outComponents = GL_R16F;
//		outFormat = GL_RED;
//		outType = GL_HALF_FLOAT;
//		break;
//
//	case TfR32F:
//		outPixelSize = 4;
//		outComponents = GL_R32F;
//		outFormat = GL_RED;
//		outType = GL_FLOAT;
//		break;
//
//	case TfDXT1:
//		outPixelSize = 0;
//		outComponents = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
//		outFormat = GL_RGBA;
//		outType = GL_UNSIGNED_BYTE;
//		break;
//
//	case TfDXT2:
//	case TfDXT3:
//		outPixelSize = 0;
//		outComponents = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
//		outFormat = GL_RGBA;
//		outType = GL_UNSIGNED_BYTE;
//		break;
//
//	case TfDXT4:
//	case TfDXT5:
//		outPixelSize = 0;
//		outComponents = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
//		outFormat = GL_RGBA;
//		outType = GL_UNSIGNED_BYTE;
//		break;
//
//#if defined(GL_IMG_texture_compression_pvrtc)
//	case TfPVRTC1:
//		outPixelSize = 0;
//		outComponents = GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG;
//		outFormat = GL_RGB;
//		outType = GL_UNSIGNED_BYTE;
//		break;
//
//	case TfPVRTC2:
//		outPixelSize = 0;
//		outComponents = GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG;
//		outFormat = GL_RGB;
//		outType = GL_UNSIGNED_BYTE;
//		break;
//
//	case TfPVRTC3:
//		outPixelSize = 0;
//		outComponents = GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
//		outFormat = GL_RGBA;
//		outType = GL_UNSIGNED_BYTE;
//		break;
//
//	case TfPVRTC4:
//		outPixelSize = 0;
//		outComponents = GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG;
//		outFormat = GL_RGBA;
//		outType = GL_UNSIGNED_BYTE;
//		break;
//#endif
//
//	case TfETC1:
//		break;
//
//	default:
//		T_DEBUG(L"Unsupported texture outFormat used");
//		return false;
//	}
//
//	return true;
//}
//
//bool convertTextureFormat_sRGB(TextureFormat textureFormat, int& outPixelSize, GLint& outComponents, GLenum& outFormat, GLenum& outType)
//{
//	switch (textureFormat)
//	{
//	case TfR8:
//#if !defined(__APPLE__)
//		outPixelSize = 1;
//		outComponents = GL_SLUMINANCE8_EXT;
//		outFormat = GL_RED;
//		outType = GL_UNSIGNED_BYTE;
//#else
//		outPixelSize = 1;
//		outComponents = GL_R8;
//		outFormat = GL_RED;
//		outType = GL_UNSIGNED_BYTE;
//#endif
//		break;
//
//	case TfR8G8B8A8:
//		outPixelSize = 4;
//		outComponents = GL_SRGB8_ALPHA8;
//		outFormat = GL_RGBA;
//		outType = GL_UNSIGNED_INT_8_8_8_8_REV;
//		break;
//
//	case TfDXT1:
//		outPixelSize = 0;
//		outComponents = GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT;
//		outFormat = GL_RGBA;
//		outType = GL_UNSIGNED_BYTE;
//		break;
//
//	case TfDXT2:
//	case TfDXT3:
//		outPixelSize = 0;
//		outComponents = GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT;
//		outFormat = GL_RGBA;
//		outType = GL_UNSIGNED_BYTE;
//		break;
//
//	case TfDXT4:
//	case TfDXT5:
//		outPixelSize = 0;
//		outComponents = GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT;
//		outFormat = GL_RGBA;
//		outType = GL_UNSIGNED_BYTE;
//		break;
//
//	default:
//		T_DEBUG(L"Unsupported texture outFormat used");
//		return false;
//	}
//
//	return true;
//}

bool convertTargetFormat(TextureFormat targetFormat, GLenum& outInternalFormat, GLint& outFormat, GLenum& outType)
{
	switch (targetFormat)
	{
	case TfR8:
#if defined(GL_RED_EXT)
		if (haveExtension("GL_EXT_texture_rg"))
		{
			outInternalFormat = GL_RED_EXT;
			outFormat = GL_RED_EXT;
			outType = GL_UNSIGNED_BYTE;
		}
		else
		{
			log::warning << L"Extension \"GL_EXT_texture_rg\" not supported; using different outFormat which may cause performance issues (TfR8 requested)." << Endl;
			outInternalFormat = GL_RGBA;
			outFormat = GL_RGBA;
			outType = GL_UNSIGNED_BYTE;
		}
#else
		outInternalFormat = GL_RGBA;
		outFormat = GL_RGBA;
		outType = GL_UNSIGNED_BYTE;
#endif
		break;

	case TfR8G8B8A8:
		outInternalFormat = GL_RGBA;
		outFormat = GL_RGBA;
		outType = GL_UNSIGNED_BYTE;
		break;

	case TfR5G6B5:
	case TfR5G5B5A1:
	case TfR4G4B4A4:
	case TfR10G10B10A2:
		outInternalFormat = GL_RGBA;
		outFormat = GL_RGBA;
		outType = GL_UNSIGNED_BYTE;
		break;

	case TfR11G11B10F:
		outInternalFormat = GL_RGBA;
		outFormat = GL_RGBA;
		outType = GL_UNSIGNED_BYTE;
		break;

#if defined(GL_HALF_FLOAT_OES)
	case TfR16G16B16A16F:
		outInternalFormat = GL_RGBA;
		outFormat = GL_RGBA;
		outType = GL_HALF_FLOAT_OES;
		break;
#endif

	case TfR32G32B32A32F:
		outInternalFormat = GL_RGBA;
		outFormat = GL_RGBA;
		outType = GL_FLOAT;
		break;

	case TfR16F:
#if !defined(__RPI__)
		if (haveExtension("GL_EXT_texture_rg"))
		{
			outInternalFormat = GL_RED_EXT;
			outFormat = GL_RED_EXT;
		}
		else
#endif
		{
			log::warning << L"Extension \"GL_EXT_texture_rg\" not supported; using different outFormat which may cause performance issues (TfR16F requested)." << Endl;
			outInternalFormat = GL_RGBA;
			outFormat = GL_RGBA;
		}
		if (haveExtension("GL_OES_texture_half_float"))
			outType = GL_HALF_FLOAT_OES;
		else
		{
			log::warning << L"Extension \"GL_OES_texture_half_float\" not supported; using different outFormat (TfR32F) which may cause performance issues (TfR16F requested)." << Endl;
			outType = GL_FLOAT;
		}
		break;

	case TfR32F:
#if !defined(__RPI__)
		if (haveExtension("GL_EXT_texture_rg"))
		{
			outInternalFormat = GL_RED_EXT;
			outFormat = GL_RED_EXT;
			outType = GL_FLOAT;
		}
		else
#endif
		{
			log::warning << L"Extension \"GL_EXT_texture_rg\" not supported; using different outFormat which may cause performance issues (TfR32F requested)." << Endl;
			outInternalFormat = GL_RGBA;
			outFormat = GL_RGBA;
			outType = GL_FLOAT;
		}
		break;
	}

	T_DEBUG(L"Unsupported target outFormat");
	return false;
}

	}
}
