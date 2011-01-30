#include "Core/Log/Log.h"
#include "Core/Math/Log2.h"
#include "Render/OpenGL/Platform.h"
#include "Render/OpenGL/IContext.h"
#include "Render/OpenGL/Std/SimpleTextureOpenGL.h"
#include "Render/OpenGL/Std/Extensions.h"
#include "Render/OpenGL/Std/UtilitiesOpenGL.h"

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

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.SimpleTextureOpenGL", SimpleTextureOpenGL, ISimpleTexture)

SimpleTextureOpenGL::SimpleTextureOpenGL(IContext* resourceContext)
:	m_resourceContext(resourceContext)
,	m_textureName(0)
,	m_width(0)
,	m_height(0)
,	m_pixelSize(0)
,	m_mipCount(0)
,	m_dataSize(0)
{
}

SimpleTextureOpenGL::~SimpleTextureOpenGL()
{
	destroy();
}

bool SimpleTextureOpenGL::create(const SimpleTextureCreateDesc& desc, GLfloat maxAnisotropy)
{
	m_width = desc.width;
	m_height = desc.height;
	
	if (!isLog2(m_width) || !isLog2(m_height))
	{
		if (!opengl_have_extension("GL_ARB_texture_non_power_of_two"))
		{
			log::error << L"Cannot create non-power-of-2 texture; not supported by OpenGL driver" << Endl;
			return false;
		}
	}

	if (!convertTextureFormat(desc.format, m_pixelSize, m_components, m_format, m_type))
		return false;

	T_OGL_SAFE(glGenTextures(1, &m_textureName));
	T_OGL_SAFE(glBindTexture(GL_TEXTURE_2D, m_textureName));
	
	if (maxAnisotropy > 0.0f)
		T_OGL_SAFE(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy));

	m_dataSize = getTextureMipPitch(desc.format, desc.width, desc.height);

	if (desc.immutable)
	{
		T_OGL_SAFE(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));

		for (int i = 0; i < desc.mipCount; ++i)
		{
			uint32_t width = getTextureMipSize(m_width, i);
			uint32_t height = getTextureMipSize(m_height, i);

			if (desc.format >= TfDXT1 && desc.format <= TfDXT5)
			{
				uint32_t mipPitch = getTextureMipPitch(desc.format, width, height);

				T_OGL_SAFE(glCompressedTexImage2D(
					GL_TEXTURE_2D,
					i,
					m_components,
					width,
					height,
					0,
					mipPitch,
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

		if (desc.mipCount > 1 && ((m_width >> desc.mipCount) == 1 || (m_height >> desc.mipCount) == 1))
		{
			log::warning << L"Creating last missing mipmap, re-import texture" << Endl;

			uint8_t dummy[32];

			T_OGL_SAFE(glTexImage2D(
				GL_TEXTURE_2D,
				desc.mipCount,
				m_components,
				1,
				1,
				0,
				m_format,
				m_type,
				dummy
			));
		}
	}

	m_mipCount = desc.mipCount;
	std::memset(&m_shadowState, 0, sizeof(m_shadowState));
	
	return true;
}

void SimpleTextureOpenGL::destroy()
{
	if (m_textureName)
	{
		if (m_resourceContext)
			m_resourceContext->deleteResource(new DeleteTextureCallback(m_textureName));
		m_textureName = 0;
	}
}

int SimpleTextureOpenGL::getWidth() const
{
	return m_width;
}

int SimpleTextureOpenGL::getHeight() const
{
	return m_height;
}

int SimpleTextureOpenGL::getDepth() const
{
	return 1;
}

bool SimpleTextureOpenGL::lock(int level, Lock& lock)
{
	if (!m_data.ptr())
	{
		m_data.reset(new uint8_t [m_dataSize]);
		if (!m_data.ptr())
			return false;
	}

	lock.pitch = (m_width >> level) * m_pixelSize;
	lock.bits = m_data.ptr();
	return true;
}

void SimpleTextureOpenGL::unlock(int level)
{
	T_ANONYMOUS_VAR(IContext::Scope)(m_resourceContext);
	T_OGL_SAFE(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
	T_OGL_SAFE(glBindTexture(GL_TEXTURE_2D, m_textureName));
	T_OGL_SAFE(glTexImage2D(
		GL_TEXTURE_2D,
		level,
		m_components,
		m_width >> level,
		m_height >> level,
		0,
		m_format,
		m_type,
		m_data.c_ptr()
	));
}

void SimpleTextureOpenGL::bind(GLuint unit, const SamplerState& samplerState, GLint locationTexture)
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
	
	T_OGL_SAFE(glUniform1iARB(locationTexture, unit));
}

	}
}
