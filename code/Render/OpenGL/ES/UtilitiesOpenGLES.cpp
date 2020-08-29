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

// https://www.khronos.org/registry/OpenGL-Refpages/es3.0/html/glTexImage2D.xhtml

bool convertTextureFormat(TextureFormat textureFormat, int& outPixelSize, GLint& outInternalFormat, GLenum& outFormat, GLenum& outType)
{
	switch (textureFormat)
	{
	case TfR8:
		outPixelSize = 1;
		outInternalFormat = GL_R8;
		outFormat = GL_RED;
		outType = GL_UNSIGNED_BYTE;
		break;

	case TfR8G8B8A8:
		outPixelSize = 4;
		outInternalFormat = GL_RGBA8;
		outFormat = GL_RGBA;
		outType = GL_UNSIGNED_BYTE;
		break;

	case TfR11G11B10F:
		outPixelSize = 4;
		outInternalFormat = GL_R11F_G11F_B10F;
		outFormat = GL_RGB;
		outType = GL_HALF_FLOAT;
		break;

	case TfR16G16B16A16F:
		outPixelSize = 8;
		outInternalFormat = GL_RGBA16F;
		outFormat = GL_RGBA;
		outType = GL_HALF_FLOAT;
		break;

	case TfR32G32B32A32F:
		outPixelSize = 16;
		outInternalFormat = GL_RGBA32F;
		outFormat = GL_RGBA;
		outType = GL_FLOAT;
		break;

	case TfR16F:
		outPixelSize = 2;
		outInternalFormat = GL_R16F;
		outFormat = GL_RED;
		outType = GL_HALF_FLOAT;
		break;

	case TfR32F:
		outPixelSize = 4;
		outInternalFormat = GL_R32F;
		outFormat = GL_RED;
		outType = GL_FLOAT;
		break;

#if defined(GL_IMG_texture_compression_pvrtc)
	case TfPVRTC1:
		outPixelSize = 0;
		outInternalFormat = GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG;
		outFormat = GL_RGBA;
		outType = GL_UNSIGNED_BYTE;
		break;

	case TfPVRTC2:
		outPixelSize = 0;
		outInternalFormat = GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG;
		outFormat = GL_RGBA;
		outType = GL_UNSIGNED_BYTE;
		break;

	case TfPVRTC3:
		outPixelSize = 0;
		outInternalFormat = GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
		outFormat = GL_RGBA;
		outType = GL_UNSIGNED_BYTE;
		break;

	case TfPVRTC4:
		outPixelSize = 0;
		outInternalFormat = GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG;
		outFormat = GL_RGBA;
		outType = GL_UNSIGNED_BYTE;
		break;
#endif

	case TfETC1:
		outPixelSize = 0;
		outInternalFormat = GL_COMPRESSED_RGB8_ETC2;
		outFormat = GL_RGB;
		outType = GL_UNSIGNED_BYTE;
		break;

	default:
		return false;
	}

	return true;
}

bool convertTargetFormat(TextureFormat targetFormat, GLenum& outInternalFormat, GLint& outFormat, GLenum& outType)
{
	switch (targetFormat)
	{
	case TfR8:
		outInternalFormat = GL_R8;
		outFormat = GL_RED;
		outType = GL_UNSIGNED_BYTE;
		break;

	case TfR8G8B8A8:
		outInternalFormat = GL_RGBA8;
		outFormat = GL_RGBA;
		outType = GL_UNSIGNED_BYTE;
		break;

	case TfR5G6B5:
		outInternalFormat = GL_RGB565;
		outFormat = GL_RGB;
		outType = GL_UNSIGNED_SHORT_5_6_5;
		break;

	case TfR5G5B5A1:
		outInternalFormat = GL_RGB5_A1;
		outFormat = GL_RGBA;
		outType = GL_UNSIGNED_SHORT_5_5_5_1;
		break;

	case TfR4G4B4A4:
		outInternalFormat = GL_RGBA4;
		outFormat = GL_RGBA;
		outType = GL_UNSIGNED_SHORT_4_4_4_4;
		break;

	case TfR10G10B10A2:
		outInternalFormat = GL_RGB10_A2;
		outFormat = GL_RGBA;
		outType = GL_UNSIGNED_INT_2_10_10_10_REV;
		break;

	case TfR11G11B10F:
		outInternalFormat = GL_R11F_G11F_B10F;
		outFormat = GL_RGB;
		outType = GL_HALF_FLOAT;
		break;

	case TfR16G16B16A16F:
		outInternalFormat = GL_RGBA16F;
		outFormat = GL_RGBA;
		outType = GL_HALF_FLOAT;
		break;

	case TfR32G32B32A32F:
		outInternalFormat = GL_RGBA32F;
		outFormat = GL_RGBA;
		outType = GL_FLOAT;
		break;

	case TfR16G16F:
		outInternalFormat = GL_RG16F;
		outFormat = GL_RG;
		outType = GL_HALF_FLOAT;
		break;

	case TfR32G32F:
		outInternalFormat = GL_RG32F;
		outFormat = GL_RG;
		outType = GL_FLOAT;
		break;

	case TfR16F:
		outInternalFormat = GL_R16F;
		outFormat = GL_RED;
		outType = GL_HALF_FLOAT;
		break;

	case TfR32F:
		outInternalFormat = GL_R32F;
		outFormat = GL_RED;
		outType = GL_FLOAT;
		break;

	default:
		return false;
	}

	return true;
}

	}
}
