#include "Render/OpenGL/IContext.h"
#include "Render/OpenGL/Std/Extensions.h"
#include "Render/OpenGL/Std/RenderTargetOpenGL.h"
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

struct DeleteFramebufferCallback : public IContext::IDeleteCallback
{
	GLuint m_framebufferName;

	DeleteFramebufferCallback(GLuint framebufferName)
	:	m_framebufferName(framebufferName)
	{
	}

	virtual void deleteResource()
	{
		T_OGL_SAFE(glDeleteFramebuffersEXT(1, &m_framebufferName));
		delete this;
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderTargetOpenGL", RenderTargetOpenGL, ITexture)

RenderTargetOpenGL::RenderTargetOpenGL(IContext* context)
:	m_context(context)
,	m_width(0)
,	m_height(0)
,	m_textureTarget(0)
,	m_frameBufferObject(0)
,	m_colorTexture(0)
,	m_haveDepth(false)
,	m_originAndScale(0.0f, 1.0f, 1.0f, -1.0f)
{
}

RenderTargetOpenGL::~RenderTargetOpenGL()
{
	destroy();
}

bool RenderTargetOpenGL::create(const RenderTargetSetCreateDesc& setDesc, const RenderTargetCreateDesc& desc, GLuint depthBuffer)
{
	GLenum internalFormat;
	GLint format;
	GLenum type;

	m_width = setDesc.width;
	m_height = setDesc.height;

	switch (desc.format)
	{
	case TfR8G8B8A8:
		internalFormat = GL_RGBA;
		format = GL_RGBA;
		type = GL_UNSIGNED_BYTE;
		m_textureTarget = GL_TEXTURE_2D;
		break;

	case TfR16G16B16A16F:
#if !defined(__APPLE__)
		internalFormat = GL_RGBA16F_ARB;
		format = GL_RGBA;
		type = GL_HALF_FLOAT_ARB;
		m_textureTarget = GL_TEXTURE_2D;
#else
		internalFormat = GL_RGBA16F_ARB;
		format = GL_RGBA;
		type = GL_FLOAT;
		m_textureTarget = GL_TEXTURE_2D;
#endif
		break;

	case TfR32G32B32A32F:
		internalFormat = GL_RGBA32F_ARB;
		format = GL_RGBA;
		type = GL_FLOAT;
		m_textureTarget = GL_TEXTURE_2D;
		break;

#if !defined(__APPLE__)
	case TfR16F:
		if (/* have GL_NV_float_buffer */ 0)
		{
			internalFormat = GL_FLOAT_R16_NV;
			format = GL_RED;
			type = GL_FLOAT;
			m_textureTarget = GL_TEXTURE_RECTANGLE_NV;
		}
		else if (/* have GL_ATI_texture_float */ 1)
		{
			internalFormat = GL_LUMINANCE_FLOAT16_ATI;
			format = GL_RED;
			type = GL_FLOAT;
			m_textureTarget = GL_TEXTURE_2D;
		}
		break;

	case TfR32F:
		if (/* have GL_NV_float_buffer */ 0)
		{
			internalFormat = GL_FLOAT_R32_NV;
			format = GL_RED;
			type = GL_FLOAT;
			m_textureTarget = GL_TEXTURE_RECTANGLE_NV;
		}
		else if (/* have GL_ATI_texture_float */ 1)
		{
			internalFormat = GL_LUMINANCE_FLOAT32_ATI;
			format = GL_RED;
			type = GL_FLOAT;
			m_textureTarget = GL_TEXTURE_2D;
		}
		break;
#else
	case TfR16F:
		internalFormat = GL_RGBA16F_ARB;
		format = GL_RGBA;
		type = GL_FLOAT;
		m_textureTarget = GL_TEXTURE_2D;
		break;
			
	case TfR32F:
		internalFormat = GL_RGBA32F_ARB;
		format = GL_RGBA;
		type = GL_FLOAT;
		m_textureTarget = GL_TEXTURE_2D;
		break;
#endif

	default:
		log::error << L"Unable to create render target, unsupported format" << Endl;
		return false;
	}

	T_OGL_SAFE(glGenFramebuffersEXT(1, &m_frameBufferObject));
	T_OGL_SAFE(glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_frameBufferObject));

	if (depthBuffer)
	{
		T_OGL_SAFE(glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, depthBuffer));
		m_haveDepth = true;
	}

	int targetWidth = m_width;
	int targetHeight = m_height;

	T_OGL_SAFE(glGenTextures(1, &m_colorTexture));

	T_OGL_SAFE(glActiveTexture(GL_TEXTURE0));
	T_OGL_SAFE(glBindTexture(m_textureTarget, m_colorTexture));

	T_OGL_SAFE(glTexParameterf(m_textureTarget, GL_TEXTURE_WRAP_S, GL_CLAMP));
	T_OGL_SAFE(glTexParameterf(m_textureTarget, GL_TEXTURE_WRAP_T, GL_CLAMP));
	T_OGL_SAFE(glTexParameterf(m_textureTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
	T_OGL_SAFE(glTexParameterf(m_textureTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST));

	T_OGL_SAFE(glTexImage2D(m_textureTarget, 0, internalFormat, targetWidth, targetHeight, 0, format, type, NULL));

	T_OGL_SAFE(glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, m_textureTarget, m_colorTexture, 0));

	GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	T_OGL_SAFE(glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0));

	return bool(status == GL_FRAMEBUFFER_COMPLETE_EXT);
}

void RenderTargetOpenGL::destroy()
{
	if (m_colorTexture)
	{
		if (m_context)
			m_context->deleteResource(new DeleteTextureCallback(m_colorTexture));
		m_colorTexture = 0;
	}
	if (m_frameBufferObject)
	{
		if (m_context)
			m_context->deleteResource(new DeleteFramebufferCallback(m_frameBufferObject));
		m_frameBufferObject = 0;
	}
}

int RenderTargetOpenGL::getWidth() const
{
	return m_width;
}

int RenderTargetOpenGL::getHeight() const
{
	return m_height;
}

int RenderTargetOpenGL::getDepth() const
{
	return 0;
}

void RenderTargetOpenGL::bind()
{
	T_OGL_SAFE(glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_frameBufferObject));
}

void RenderTargetOpenGL::enter(bool keepDepthStencil)
{
	T_OGL_SAFE(glViewport(0, 0, m_width, m_height));

	if (m_haveDepth || keepDepthStencil)
	{
		T_OGL_SAFE(glEnable(GL_DEPTH_TEST));
		T_OGL_SAFE(glDepthFunc(GL_LEQUAL));
		T_OGL_SAFE(glDepthMask(GL_TRUE));
	}
	else
	{
		T_OGL_SAFE(glDisable(GL_DEPTH_TEST));
		T_OGL_SAFE(glDepthMask(GL_FALSE));
	}

	T_OGL_SAFE(glActiveTexture(GL_TEXTURE0));
	T_OGL_SAFE(glBindTexture(m_textureTarget, m_colorTexture));
}

	}
}
