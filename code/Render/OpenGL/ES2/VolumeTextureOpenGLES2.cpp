#include <cstring>
#include "Core/Log/Log.h"
#include "Render/OpenGL/Platform.h"
#include "Render/OpenGL/ES2/VolumeTextureOpenGLES2.h"
#if defined(__ANDROID__)
#	include "Render/OpenGL/ES2/Android/ContextOpenGLES2.h"
#elif defined(__IOS__)
#	include "Render/OpenGL/ES2/iOS/EAGLContextWrapper.h"
#elif defined(__EMSCRIPTEN__)
#	include "Render/OpenGL/ES2/Emscripten/ContextOpenGLES2.h"
#elif defined(__PNACL__)
#	include "Render/OpenGL/ES2/PNaCl/ContextOpenGLES2.h"
#elif defined(_WIN32)
#	include "Render/OpenGL/ES2/Win32/ContextOpenGLES2.h"
#endif

namespace traktor
{
	namespace render
	{
		namespace
		{

struct DeleteTextureCallback : public ContextOpenGLES2::IDeleteCallback
{
	GLuint m_textureName;

	DeleteTextureCallback(GLuint textureName)
	:	m_textureName(textureName)
	{
	}

	virtual ~DeleteTextureCallback()
	{
	}

	virtual void deleteResource()
	{
		T_OGL_SAFE(glDeleteTextures(1, &m_textureName));
		delete this;
	}
};

bool convertTextureFormat(TextureFormat textureFormat, int& outPixelSize, GLint& outComponents, GLenum& outFormat, GLenum& outType)
{
	switch (textureFormat)
	{
#if defined(GL_RED_EXT)
	case TfR8:
		outPixelSize = 1;
		outComponents = GL_RED_EXT;
		outFormat = GL_RED_EXT;
		outType = GL_UNSIGNED_BYTE;
		break;
#endif

	case TfR8G8B8A8:
		outPixelSize = 4;
		outComponents = GL_RGBA;
		outFormat = GL_RGBA;
		outType = GL_UNSIGNED_BYTE;
		break;

	case TfR32G32B32A32F:
		outPixelSize = 16;
		outComponents = 4;
		outFormat = GL_RGBA;
		outType = GL_FLOAT;
		break;

#if defined(GL_RED_EXT)
	case TfR16F:
		outPixelSize = 2;
		outComponents = GL_RED_EXT;
		outFormat = GL_RED_EXT;
		outType = GL_HALF_FLOAT_OES;
		break;
#endif

#if defined(GL_RED_EXT)
	case TfR32F:
		outPixelSize = 4;
		outComponents = GL_RED_EXT;
		outFormat = GL_RED_EXT;
		outType = GL_FLOAT;
		break;
#endif

#if defined(GL_IMG_texture_compression_pvrtc)
	case TfPVRTC1:
		outPixelSize = 0;
		outComponents = GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG;
		outFormat = GL_RGBA;
		outType = GL_UNSIGNED_BYTE;
		break;

	case TfPVRTC2:
		outPixelSize = 0;
		outComponents = GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG;
		outFormat = GL_RGBA;
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

	case TfETC1:
		break;

	default:
		return false;
	}

	return true;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.VolumeTextureOpenGLES2", VolumeTextureOpenGLES2, IVolumeTexture)

VolumeTextureOpenGLES2::VolumeTextureOpenGLES2(ContextOpenGLES2* resourceContext)
:	m_resourceContext(resourceContext)
,	m_textureName(0)
,	m_width(0)
,	m_height(0)
,	m_depth(0)
,	m_pixelSize(0)
,	m_mipCount(0)
,	m_components(0)
,	m_format(0)
,	m_type(0)
{
}

VolumeTextureOpenGLES2::~VolumeTextureOpenGLES2()
{
	destroy();
}

bool VolumeTextureOpenGLES2::create(const VolumeTextureCreateDesc& desc)
{
	m_width = desc.width;
	m_height = desc.height;
	m_depth = desc.depth;
	m_mipCount = desc.mipCount;

	convertTextureFormat(desc.format, m_pixelSize, m_components, m_format, m_type);

#if !defined(_WIN32) && !defined(__IOS__) && !defined(__PNACL__) && !defined(__EMSCRIPTEN__)

	T_OGL_SAFE(glGenTextures(1, &m_textureName));

	if (desc.immutable)
	{
		T_OGL_SAFE(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
		T_OGL_SAFE(glBindTexture(GL_TEXTURE_3D_OES, m_textureName));

		if (desc.format >= TfPVRTC1 && desc.format <= TfPVRTC4)
		{
#	if defined(__APPLE__)
			uint32_t mipPitch = getTextureMipPitch(desc.format, m_width, m_height);
			T_OGL_SAFE(glCompressedTexImage3DOES(
				GL_TEXTURE_3D_OES,
				0,
				m_components,
				m_width,
				m_height,
				m_depth,
				0,
				mipPitch,
				desc.initialData[0].data
			));
#	else
			log::error << L"Compressed 3D textures not supported on non-iOS platforms" << Endl;
			return false;
#	endif
		}
		else
		{
			T_OGL_SAFE(glTexImage3DOES(
				GL_TEXTURE_3D_OES,
				0,
				m_components,
				m_width,
				m_height,
				m_depth,
				0,
				m_format,
				m_type,
				desc.initialData[0].data
			));
		}
	}

#endif

	return true;
}

void VolumeTextureOpenGLES2::destroy()
{
	if (m_textureName)
	{
		m_resourceContext->deleteResource(new DeleteTextureCallback(m_textureName));
		m_textureName = 0;
	}
}

ITexture* VolumeTextureOpenGLES2::resolve()
{
	return this;
}

int VolumeTextureOpenGLES2::getWidth() const
{
	return m_width;
}

int VolumeTextureOpenGLES2::getHeight() const
{
	return m_height;
}

int VolumeTextureOpenGLES2::getDepth() const
{
	return m_depth;
}

void VolumeTextureOpenGLES2::bindSampler(GLuint unit, const SamplerStateOpenGL& samplerState, GLint locationTexture)
{
#if !defined(__IOS__) && !defined(__PNACL__)

	T_OGL_SAFE(glActiveTexture(GL_TEXTURE0 + unit));
	T_OGL_SAFE(glBindTexture(GL_TEXTURE_3D_OES, m_textureName));

	GLenum minFilter = GL_NEAREST;
	if (m_mipCount > 1)
		minFilter = samplerState.minFilter;

	if (m_shadowState.minFilter != minFilter)
	{
		T_OGL_SAFE(glTexParameteri(GL_TEXTURE_3D_OES, GL_TEXTURE_MIN_FILTER, minFilter));
		m_shadowState.minFilter = minFilter;
	}

	if (m_shadowState.magFilter != samplerState.magFilter)
	{
		T_OGL_SAFE(glTexParameteri(GL_TEXTURE_3D_OES, GL_TEXTURE_MAG_FILTER, samplerState.magFilter));
		m_shadowState.magFilter = samplerState.magFilter;
	}

	if (m_shadowState.wrapS != samplerState.wrapS)
	{
		T_OGL_SAFE(glTexParameteri(GL_TEXTURE_3D_OES, GL_TEXTURE_WRAP_S, samplerState.wrapS));
		m_shadowState.wrapS = samplerState.wrapS;
	}

	if (m_shadowState.wrapT != samplerState.wrapT)
	{
		T_OGL_SAFE(glTexParameteri(GL_TEXTURE_3D_OES, GL_TEXTURE_WRAP_T, samplerState.wrapT));
		m_shadowState.wrapT = samplerState.wrapT;
	}

	T_OGL_SAFE(glUniform1i(locationTexture, unit));

#endif
}

void VolumeTextureOpenGLES2::bindSize(GLint locationSize)
{
	T_OGL_SAFE(glUniform4f(locationSize, GLfloat(m_width), GLfloat(m_height), GLfloat(m_depth), GLfloat(1.0f)));
}

	}
}
