#include <algorithm>
#include <cstring>
#include "Core/Log/Log.h"
#include "Core/Math/Log2.h"
#include "Render/OpenGL/Platform.h"
#include "Render/OpenGL/ES2/ContextOpenGLES2.h"
#include "Render/OpenGL/ES2/SimpleTextureOpenGLES2.h"

#if !defined(T_OFFLINE_ONLY)

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
#elif defined(__ANDROID__)
	case TfR8:
		outPixelSize = 1;
		outComponents = GL_LUMINANCE;
		outFormat = GL_LUMINANCE;
		outType = GL_UNSIGNED_BYTE;
		break;
#endif

	case TfR8G8B8A8:
		outPixelSize = 4;
		outComponents = GL_RGBA;
		outFormat = GL_RGBA;
		outType = GL_UNSIGNED_BYTE;
		break;

#if defined(GL_RED_EXT)
	case TfR16G16B16A16F:
		outPixelSize = 8;
		outComponents = GL_RGBA;
		outFormat = GL_RGBA;
		outType = GL_HALF_FLOAT_OES;
		break;
#endif

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

#if defined(GL_OES_compressed_ETC1_RGB8_texture)
	case TfETC1:
		outPixelSize = 0;
		outComponents = GL_OES_compressed_ETC1_RGB8_texture;
		outFormat = GL_RGBA;
		outType = GL_UNSIGNED_BYTE;
		break;
#endif

	default:
		return false;
	}

	return true;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.SimpleTextureOpenGLES2", SimpleTextureOpenGLES2, ISimpleTexture)

SimpleTextureOpenGLES2::SimpleTextureOpenGLES2(ContextOpenGLES2* context)
:	m_context(context)
,	m_textureName(0)
,	m_pot(false)
,	m_dirty(-1)
,	m_width(0)
,	m_height(0)
,	m_pixelSize(0)
,	m_mipCount(0)
{
	std::memset(&m_shadowState, 0, sizeof(m_shadowState));
}

SimpleTextureOpenGLES2::~SimpleTextureOpenGLES2()
{
	destroy();
}

bool SimpleTextureOpenGLES2::create(const SimpleTextureCreateDesc& desc)
{
	m_pot = isLog2(desc.width) && isLog2(desc.height);
	m_width = desc.width;
	m_height = desc.height;

	if (!convertTextureFormat(desc.format, m_pixelSize, m_components, m_format, m_type))
	{
		log::error << L"Unable to convert texture format" << Endl;
		return false;
	}

	T_OGL_SAFE(glGenTextures(1, &m_textureName));
	T_OGL_SAFE(glBindTexture(GL_TEXTURE_2D, m_textureName));
	T_OGL_SAFE(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));

	// Set default parameters as its might help driver.
	T_OGL_SAFE(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	T_OGL_SAFE(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	if (m_pot)
	{
		T_OGL_SAFE(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
		T_OGL_SAFE(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
	}
	else
	{
		T_OGL_SAFE(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
		T_OGL_SAFE(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));		
	}

	// Allocate data buffer.
	uint32_t texturePitch = getTextureMipPitch(desc.format, desc.width, desc.height);
	if (desc.immutable)
	{
		for (int i = 0; i < desc.mipCount; ++i)
		{
			uint32_t width = std::max(m_width >> i, 1);
			uint32_t height = std::max(m_height >> i, 1);

			if (
				(desc.format >= TfDXT1 && desc.format <= TfDXT5) ||
				(desc.format >= TfPVRTC1 && desc.format <= TfPVRTC4) ||
				desc.format == TfETC1
			)
			{
				uint32_t mipSize = getTextureMipPitch(desc.format, width, height);
				T_OGL_SAFE(glCompressedTexImage2D(
					GL_TEXTURE_2D,
					i,
					m_components,
					width,
					height,
					0,
					mipSize,
					desc.initialData[i].data
				));
			}
			else
			{
				T_OGL_SAFE(glTexImage2D(
					GL_TEXTURE_2D,
					i,
					m_components,
					width,
					height,
					0,
					m_format,
					m_type,
					desc.initialData[i].data
				));
			}
		}
	}
	else
	{
		m_data.resize(texturePitch, 0);

		for (int i = 0; i < desc.mipCount; ++i)
		{
			uint32_t width = std::max(m_width >> i, 1);
			uint32_t height = std::max(m_height >> i, 1);

			T_OGL_SAFE(glTexImage2D(
				GL_TEXTURE_2D,
				i,
				m_components,
				width,
				height,
				0,
				m_format,
				m_type,
				desc.initialData[i].data ? desc.initialData[i].data : m_data.c_ptr()
			));
		}
	}

	m_mipCount = desc.mipCount;
	return true;
}

void SimpleTextureOpenGLES2::destroy()
{
	if (m_textureName)
	{
		if (m_context)
			m_context->deleteResource(new DeleteTextureCallback(m_textureName));
		m_textureName = 0;
	}
}

ITexture* SimpleTextureOpenGLES2::resolve()
{
	return this;
}

int SimpleTextureOpenGLES2::getWidth() const
{
	return m_width;
}

int SimpleTextureOpenGLES2::getHeight() const
{
	return m_height;
}

bool SimpleTextureOpenGLES2::lock(int level, Lock& lock)
{
	if (m_data.empty() || level >= m_mipCount)
		return false;

	lock.pitch = std::max(m_width >> level, 1) * m_pixelSize;
	lock.bits = &m_data[0];
	return true;
}

void SimpleTextureOpenGLES2::unlock(int level)
{
	m_dirty = level;
}

void* SimpleTextureOpenGLES2::getInternalHandle()
{
	return (void*)m_textureName;
}

void SimpleTextureOpenGLES2::bindSampler(GLuint unit, const SamplerStateOpenGL& samplerState, GLint locationTexture)
{
	T_OGL_SAFE(glActiveTexture(GL_TEXTURE0 + unit));
	T_OGL_SAFE(glBindTexture(GL_TEXTURE_2D, m_textureName));

	if (m_dirty >= 0)
	{
		T_OGL_SAFE(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
		T_OGL_SAFE(glTexSubImage2D(
			GL_TEXTURE_2D,
			m_dirty,
			0,
			0,
			std::max(m_width >> m_dirty, 1),
			std::max(m_height >> m_dirty, 1),
			m_format,
			m_type,
			m_data.c_ptr()
		));
		m_dirty = -1;
	}

	GLenum minFilter = GL_NEAREST;
	if (m_mipCount > 1)
		minFilter = samplerState.minFilter;

	if (m_shadowState.minFilter != minFilter)
	{
		T_OGL_SAFE(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter));
		m_shadowState.minFilter = minFilter;
	}

	if (m_shadowState.magFilter != samplerState.magFilter)
	{
		T_OGL_SAFE(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, samplerState.magFilter));
		m_shadowState.magFilter = samplerState.magFilter;
	}

	if (m_pot)
	{
		if (m_shadowState.wrapS != samplerState.wrapS)
		{
			T_OGL_SAFE(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, samplerState.wrapS));
			m_shadowState.wrapS = samplerState.wrapS;
		}

		if (m_shadowState.wrapT != samplerState.wrapT)
		{
			T_OGL_SAFE(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, samplerState.wrapT));
			m_shadowState.wrapT = samplerState.wrapT;
		}
	}

	T_OGL_SAFE(glUniform1i(locationTexture, unit));
}

void SimpleTextureOpenGLES2::bindSize(GLint locationSize)
{
	T_OGL_SAFE(glUniform4f(locationSize, GLfloat(m_width), GLfloat(m_height), GLfloat(0), GLfloat(0)));
}

	}
}

#endif
