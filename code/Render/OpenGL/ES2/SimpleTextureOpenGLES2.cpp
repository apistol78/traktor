#include "Render/OpenGL/Platform.h"
#include "Render/OpenGL/IContext.h"
#include "Render/OpenGL/ES2/SimpleTextureOpenGLES2.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

struct DeleteTextureCallback : public IContext::IDeleteCallback
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
	//case TfR8:
	//	outPixelSize = 1;
	//	outComponents = 1;
	//	outFormat = GL_RED;
	//	outType = GL_UNSIGNED_BYTE;
	//	break;

	case TfR8G8B8A8:
		outPixelSize = 4;
		outComponents = GL_RGBA;
		outFormat = GL_RGBA;
		outType = GL_UNSIGNED_BYTE;
		break;

	//case TfR16G16B16A16F:
	//	break;

	case TfR32G32B32A32F:
		outPixelSize = 16;
		outComponents = 4;
		outFormat = GL_RGBA;
		outType = GL_FLOAT;
		break;

	//case TfR16F:
	//	outPixelSize = 2;
	//	outComponents = 1;
	//	outFormat = GL_RED;
	//	outType = GL_HALF_FLOAT_ARB;
	//	break;

	//case TfR32F:
	//	outPixelSize = 4;
	//	outComponents = 1;
	//	outFormat = GL_RED;
	//	outType = GL_FLOAT;
	//	break;

	//case TfDXT1:
	//	outPixelSize = 0;
	//	outComponents = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
	//	outFormat = GL_RGBA;
	//	outType = GL_UNSIGNED_BYTE;
	//	break;

	//case TfDXT3:
	//	outPixelSize = 0;
	//	outComponents = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
	//	outFormat = GL_RGBA;
	//	outType = GL_UNSIGNED_BYTE;
	//	break;

	default:
		return false;
	}

	return true;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.SimpleTextureOpenGLES2", SimpleTextureOpenGLES2, ISimpleTexture)

SimpleTextureOpenGLES2::SimpleTextureOpenGLES2(IContext* context)
:	m_context(context)
,	m_textureName(0)
,	m_width(0)
,	m_height(0)
,	m_pixelSize(0)
,	m_mipCount(0)
{
}

SimpleTextureOpenGLES2::~SimpleTextureOpenGLES2()
{
	destroy();
}

bool SimpleTextureOpenGLES2::create(const SimpleTextureCreateDesc& desc)
{
	m_width = desc.width;
	m_height = desc.height;

	if (!convertTextureFormat(desc.format, m_pixelSize, m_components, m_format, m_type))
	{
		log::error << L"Unable to convert texture format" << Endl;
		return false;
	}

	T_OGL_SAFE(glGenTextures(1, &m_textureName));

	// Allocate data buffer.
	uint32_t texturePitch = getTextureMipPitch(desc.format, desc.width, desc.height);
	if (desc.immutable)
	{
		T_OGL_SAFE(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
		T_OGL_SAFE(glBindTexture(GL_TEXTURE_2D, m_textureName));

		for (int i = 0; i < desc.mipCount; ++i)
		{
			uint32_t width = std::max(m_width >> i, 1);
			uint32_t height = std::max(m_height >> i, 1);

			if (desc.format >= TfDXT1 && desc.format <= TfDXT5)
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
		m_data.resize(texturePitch);

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

int SimpleTextureOpenGLES2::getWidth() const
{
	return m_width;
}

int SimpleTextureOpenGLES2::getHeight() const
{
	return m_height;
}

int SimpleTextureOpenGLES2::getDepth() const
{
	return 1;
}

bool SimpleTextureOpenGLES2::lock(int level, Lock& lock)
{
	if (m_data.empty())
		return false;

	lock.pitch = std::max(m_width >> level, 1) * m_pixelSize;
	lock.bits = &m_data[0];
	return true;
}

void SimpleTextureOpenGLES2::unlock(int level)
{
	T_ANONYMOUS_VAR(IContext::Scope)(m_context);
	T_OGL_SAFE(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
	T_OGL_SAFE(glBindTexture(GL_TEXTURE_2D, m_textureName));
	T_OGL_SAFE(glTexImage2D(
		GL_TEXTURE_2D,
		0,
		m_components,
		std::max(m_width >> level, 1),
		std::max(m_height >> level, 1),
		0,
		m_format,
		m_type,
		&m_data[0]
	));
}

void SimpleTextureOpenGLES2::bind(GLuint unit, const SamplerState& samplerState, GLint locationTexture, GLint locationOffset)
{
	T_OGL_SAFE(glActiveTexture(GL_TEXTURE0 + unit));
	T_OGL_SAFE(glBindTexture(GL_TEXTURE_2D, m_textureName));

	GLenum minFilter = GL_NEAREST;
	if (m_mipCount > 1)
		minFilter = samplerState.minFilter;
	else
	{
		if (samplerState.minFilter != GL_NEAREST)
			minFilter = GL_LINEAR;
		else
			minFilter = GL_NEAREST;
	}

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

	T_OGL_SAFE(glUniform1i(locationTexture, unit));
}

	}
}
