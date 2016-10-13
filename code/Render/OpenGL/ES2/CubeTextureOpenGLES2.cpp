#include <cstring>
#include "Render/OpenGL/Platform.h"
#include "Render/OpenGL/ES2/CubeTextureOpenGLES2.h"
#if defined(__ANDROID__)
#	include "Render/OpenGL/ES2/Android/ContextOpenGLES2.h"
#elif defined(__IOS__)
#	include "Render/OpenGL/ES2/iOS/ContextOpenGLES2.h"
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

const GLenum c_cubeFaces[] =
{
	GL_TEXTURE_CUBE_MAP_POSITIVE_X,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
};

struct DeleteTextureCallback : public ContextOpenGLES2::IDeleteCallback
{
	GLuint m_textureName;

	DeleteTextureCallback(GLuint textureName)
	:	m_textureName(textureName)
	{
	}

	virtual void deleteResource() T_OVERRIDE T_FINAL
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

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.CubeTextureOpenGLES2", CubeTextureOpenGLES2, ICubeTexture)

CubeTextureOpenGLES2::CubeTextureOpenGLES2(ContextOpenGLES2* resourceContext)
:	m_resourceContext(resourceContext)
,	m_textureName(0)
,	m_side(0)
,	m_pixelSize(0)
,	m_mipCount(0)
,	m_components(0)
,	m_format(0)
,	m_type(0)
{
}

CubeTextureOpenGLES2::~CubeTextureOpenGLES2()
{
	destroy();
}

bool CubeTextureOpenGLES2::create(const CubeTextureCreateDesc& desc)
{
	m_side = desc.side;

	convertTextureFormat(desc.format, m_pixelSize, m_components, m_format, m_type);

	T_OGL_SAFE(glGenTextures(1, &m_textureName));

	m_data.reset(new uint8_t [m_side * m_side * m_pixelSize]);

	if (desc.immutable)
	{
		T_OGL_SAFE(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
		T_OGL_SAFE(glBindTexture(GL_TEXTURE_CUBE_MAP, m_textureName));

		for (int face = 0; face < 6; ++face)
		{
			for (int i = 0; i < desc.mipCount; ++i)
			{
				uint32_t side = getTextureMipSize(m_side, i);
				if (desc.format >= TfPVRTC1 && desc.format <= TfPVRTC4)
				{
					uint32_t mipPitch = getTextureMipPitch(desc.format, side, side);
					T_OGL_SAFE(glCompressedTexImage2D(
						c_cubeFaces[face],
						i,
						m_components,
						side,
						side,
						0,
						mipPitch,
						desc.initialData[face * desc.mipCount + i].data
					));
				}
				else
				{
					T_OGL_SAFE(glTexImage2D(
						c_cubeFaces[face],
						i,
						m_components,
						side,
						side,
						0,
						m_format,
						m_type,
						desc.initialData[face * desc.mipCount + i].data
					));
				}
			}
		}
	}

	m_mipCount = desc.mipCount;
	return true;
}

void CubeTextureOpenGLES2::destroy()
{
	if (m_textureName)
	{
		m_resourceContext->deleteResource(new DeleteTextureCallback(m_textureName));
		m_textureName = 0;
	}
}

ITexture* CubeTextureOpenGLES2::resolve()
{
	return this;
}

int CubeTextureOpenGLES2::getWidth() const
{
	return m_side;
}

int CubeTextureOpenGLES2::getHeight() const
{
	return m_side;
}

int CubeTextureOpenGLES2::getDepth() const
{
	return 1;
}

bool CubeTextureOpenGLES2::lock(int side, int level, Lock& lock)
{
	lock.pitch = (m_side >> level) * m_pixelSize;
	lock.bits = m_data.ptr();
	return true;
}

void CubeTextureOpenGLES2::unlock(int side, int level)
{
}

void CubeTextureOpenGLES2::bindSampler(GLuint unit, const SamplerStateOpenGL& samplerState, GLint locationTexture)
{
	T_OGL_SAFE(glActiveTexture(GL_TEXTURE0 + unit));
	T_OGL_SAFE(glBindTexture(GL_TEXTURE_CUBE_MAP, m_textureName));

	GLenum minFilter = GL_NEAREST;
	if (m_mipCount > 1)
		minFilter = samplerState.minFilter;

	//if (m_shadowState.minFilter != minFilter)
	{
		T_OGL_SAFE(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, minFilter));
		m_shadowState.minFilter = minFilter;
	}

	//if (m_shadowState.magFilter != samplerState.magFilter)
	{
		T_OGL_SAFE(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, samplerState.magFilter));
		m_shadowState.magFilter = samplerState.magFilter;
	}

	//if (m_shadowState.wrapS != samplerState.wrapS)
	{
		T_OGL_SAFE(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, samplerState.wrapS));
		m_shadowState.wrapS = samplerState.wrapS;
	}

	//if (m_shadowState.wrapT != samplerState.wrapT)
	{
		T_OGL_SAFE(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, samplerState.wrapT));
		m_shadowState.wrapT = samplerState.wrapT;
	}

	T_OGL_SAFE(glUniform1i(locationTexture, unit));
}

void CubeTextureOpenGLES2::bindSize(GLint locationSize)
{
	T_OGL_SAFE(glUniform4f(locationSize, GLfloat(m_side), GLfloat(m_side), GLfloat(m_side), GLfloat(1.0f)));
}

	}
}
