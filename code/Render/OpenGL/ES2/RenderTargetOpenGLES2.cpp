#include "Render/OpenGL/Platform.h"
#include "Render/OpenGL/IContext.h"
#include "Render/OpenGL/ES2/RenderTargetOpenGLES2.h"
#include "Core/Log/Log.h"

#if !defined(T_OFFLINE_ONLY)

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
		T_OGL_SAFE(glDeleteFramebuffers(1, &m_framebufferName));
		delete this;
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderTargetOpenGLES2", RenderTargetOpenGLES2, ITexture)

RenderTargetOpenGLES2::RenderTargetOpenGLES2(IContext* context)
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

RenderTargetOpenGLES2::~RenderTargetOpenGLES2()
{
	destroy();
}

bool RenderTargetOpenGLES2::create(const RenderTargetSetCreateDesc& setDesc, const RenderTargetCreateDesc& desc, GLuint depthBuffer)
{
	GLenum internalFormat;
	GLint format;
	GLenum type;

	m_width = setDesc.width;
	m_height = setDesc.height;

	switch (desc.format)
	{
	case TfR8:
	case TfR8G8B8A8:
		internalFormat = GL_RGBA;
		format = GL_RGBA;
		type = GL_UNSIGNED_BYTE;
		m_textureTarget = GL_TEXTURE_2D;
		break;

	//case TfR16G16B16A16F:
	//	internalFormat = GL_RGBA16F;
	//	format = GL_RGBA;
	//	type = GL_HALF_FLOAT_ARB;
	//	m_textureTarget = GL_TEXTURE_2D;
	//	break;

	//case TfR32G32B32A32F:
	//	internalFormat = GL_RGBA32F;
	//	format = GL_RGBA;
	//	type = GL_FLOAT;
	//	m_textureTarget = GL_TEXTURE_2D;
	//	break;

	//case TfR16F:
	//	internalFormat = GL_RGBA16F_ARB;
	//	format = GL_RGBA;
	//	type = GL_FLOAT;
	//	m_textureTarget = GL_TEXTURE_2D;
	//	break;
	//		
	//case TfR32F:
	//	internalFormat = GL_RGBA32F_ARB;
	//	format = GL_RGBA;
	//	type = GL_FLOAT;
	//	m_textureTarget = GL_TEXTURE_2D;
	//	break;

	default:
		log::error << L"Unable to create render target, unsupported format" << Endl;
		return false;
	}

	T_OGL_SAFE(glGenFramebuffers(1, &m_frameBufferObject));
	T_OGL_SAFE(glBindFramebuffer(GL_FRAMEBUFFER, m_frameBufferObject));

	if (depthBuffer)
	{
		T_OGL_SAFE(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer));
		m_haveDepth = true;
	}

	int targetWidth = m_width;
	int targetHeight = m_height;

	T_OGL_SAFE(glGenTextures(1, &m_colorTexture));

	T_OGL_SAFE(glActiveTexture(GL_TEXTURE0));
	T_OGL_SAFE(glBindTexture(m_textureTarget, m_colorTexture));

	T_OGL_SAFE(glTexParameterf(m_textureTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	T_OGL_SAFE(glTexParameterf(m_textureTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	T_OGL_SAFE(glTexParameterf(m_textureTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
	T_OGL_SAFE(glTexParameterf(m_textureTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST));

	T_OGL_SAFE(glTexImage2D(m_textureTarget, 0, internalFormat, targetWidth, targetHeight, 0, format, type, NULL));

	T_OGL_SAFE(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_textureTarget, m_colorTexture, 0));

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	T_OGL_SAFE(glBindFramebuffer(GL_FRAMEBUFFER, 0));

	return bool(status == GL_FRAMEBUFFER_COMPLETE);
}

void RenderTargetOpenGLES2::destroy()
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

int RenderTargetOpenGLES2::getWidth() const
{
	return m_width;
}

int RenderTargetOpenGLES2::getHeight() const
{
	return m_height;
}

int RenderTargetOpenGLES2::getDepth() const
{
	return 0;
}

void RenderTargetOpenGLES2::bind(GLuint unit, const SamplerState& samplerState, GLint locationTexture, GLint locationOffset)
{
	T_OGL_SAFE(glActiveTexture(GL_TEXTURE0 + unit));
	T_OGL_SAFE(glBindTexture(m_textureTarget, m_colorTexture));

	GLenum minFilter = GL_NEAREST;
	if (samplerState.minFilter != GL_NEAREST)
		minFilter = GL_LINEAR;
	else
		minFilter = GL_NEAREST;

	if (m_shadowState.minFilter != minFilter)
	{
		T_OGL_SAFE(glTexParameteri(m_textureTarget, GL_TEXTURE_MIN_FILTER, minFilter));
		m_shadowState.minFilter = minFilter;
	}

	if (m_shadowState.magFilter != samplerState.magFilter)
	{
		T_OGL_SAFE(glTexParameteri(m_textureTarget, GL_TEXTURE_MAG_FILTER, samplerState.magFilter));
		m_shadowState.magFilter = samplerState.magFilter;
	}

	if (m_shadowState.wrapS != samplerState.wrapS)
	{
		T_OGL_SAFE(glTexParameteri(m_textureTarget, GL_TEXTURE_WRAP_S, samplerState.wrapS));
		m_shadowState.wrapS = samplerState.wrapS;
	}

	if (m_shadowState.wrapT != samplerState.wrapT)
	{
		T_OGL_SAFE(glTexParameteri(m_textureTarget, GL_TEXTURE_WRAP_T, samplerState.wrapT));
		m_shadowState.wrapT = samplerState.wrapT;
	}

	T_OGL_SAFE(glUniform1i(locationTexture, unit));

	if (locationOffset != -1)
		T_OGL_SAFE(glUniform4fv(locationOffset, 1, (const GLfloat*)&m_originAndScale));
}

void RenderTargetOpenGLES2::bind()
{
	T_OGL_SAFE(glBindFramebuffer(GL_FRAMEBUFFER, m_frameBufferObject));
}

void RenderTargetOpenGLES2::enter()
{
	T_OGL_SAFE(glViewport(0, 0, m_width, m_height));

	if (m_haveDepth)
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

#endif
