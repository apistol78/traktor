#include "Core/Log/Log.h"
#include "Render/OpenGL/Platform.h"
#include "Render/OpenGL/IContext.h"
#include "Render/OpenGL/ES2/RenderTargetOpenGLES2.h"

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

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderTargetOpenGLES2", RenderTargetOpenGLES2, ISimpleTexture)

RenderTargetOpenGLES2::RenderTargetOpenGLES2(IContext* context)
:	m_context(context)
,	m_width(0)
,	m_height(0)
,	m_usingPrimaryDepthBuffer(false)
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

bool RenderTargetOpenGLES2::create(const RenderTargetSetCreateDesc& setDesc, const RenderTargetCreateDesc& desc, GLuint depthBuffer, GLuint stencilBuffer)
{
	GLenum internalFormat;
	GLint format;
	GLenum type;

	m_width = setDesc.width;
	m_height = setDesc.height;
	m_usingPrimaryDepthBuffer = setDesc.usingPrimaryDepthStencil;

	switch (desc.format)
	{
	case TfR8:
#if defined(GL_RED_EXT)
		if (haveExtension("GL_EXT_texture_rg"))
		{
			internalFormat = GL_RED_EXT;
			format = GL_RED_EXT;
			type = GL_UNSIGNED_BYTE;
		}
		else
		{
			log::warning << L"Extension \"GL_EXT_texture_rg\" not supported; using different format which may cause performance issues" << Endl;
			internalFormat = GL_RGBA;
			format = GL_RGBA;
			type = GL_UNSIGNED_BYTE;
		}
#else
		internalFormat = GL_RGBA;
		format = GL_RGBA;
		type = GL_UNSIGNED_BYTE;
#endif
		m_textureTarget = GL_TEXTURE_2D;
		break;

	case TfR8G8B8A8:
		internalFormat = GL_RGBA;
		format = GL_RGBA;
		type = GL_UNSIGNED_BYTE;
		m_textureTarget = GL_TEXTURE_2D;
		break;
		
#if defined(GL_HALF_FLOAT_OES)
	case TfR16G16B16A16F:
		internalFormat = GL_RGBA;
		format = GL_RGBA;
		type = GL_HALF_FLOAT_OES;
		m_textureTarget = GL_TEXTURE_2D;
		break;
#endif
		
	case TfR32G32B32A32F:
		internalFormat = GL_RGBA;
		format = GL_RGBA;
		type = GL_FLOAT;
		m_textureTarget = GL_TEXTURE_2D;
		break;
		
#if defined(GL_HALF_FLOAT_OES) && defined(GL_RED_EXT)
	case TfR16F:
		if (haveExtension("GL_EXT_texture_rg"))
		{
			internalFormat = GL_RED_EXT;
			format = GL_RED_EXT;
			type = GL_HALF_FLOAT_OES;
			m_textureTarget = GL_TEXTURE_2D;
		}
		else
		{
			log::warning << L"Extension \"GL_EXT_texture_rg\" not supported; using different format which may cause performance issues" << Endl;
			internalFormat = GL_RGBA;
			format = GL_RGBA;
			type = GL_HALF_FLOAT_OES;
			m_textureTarget = GL_TEXTURE_2D;
		}
		break;

	case TfR32F:
		if (haveExtension("GL_EXT_texture_rg"))
		{
			internalFormat = GL_RED_EXT;
			format = GL_RED_EXT;
			type = GL_FLOAT;
			m_textureTarget = GL_TEXTURE_2D;
		}
		else
		{
			log::warning << L"Extension \"GL_EXT_texture_rg\" not supported; using different format which may cause performance issues" << Endl;
			internalFormat = GL_RGBA;
			format = GL_RGBA;
			type = GL_FLOAT;
			m_textureTarget = GL_TEXTURE_2D;
		}
		break;
#endif

	default:
		log::error << L"Unable to create render target, unsupported format" << Endl;
		return false;
	}

	T_OGL_SAFE(glGenTextures(1, &m_colorTexture));

	T_OGL_SAFE(glActiveTexture(GL_TEXTURE0));
	T_OGL_SAFE(glBindTexture(m_textureTarget, m_colorTexture));

	T_OGL_SAFE(glTexParameterf(m_textureTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	T_OGL_SAFE(glTexParameterf(m_textureTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	T_OGL_SAFE(glTexParameterf(m_textureTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
	T_OGL_SAFE(glTexParameterf(m_textureTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST));

	T_OGL_SAFE(glTexImage2D(
		m_textureTarget,
		0,
		internalFormat,
		m_width,
		m_height,
		0,
		format,
		type,
		NULL
	));

	T_OGL_SAFE(glGenFramebuffers(1, &m_frameBufferObject));
	T_OGL_SAFE(glBindFramebuffer(GL_FRAMEBUFFER, m_frameBufferObject));

	T_OGL_SAFE(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_textureTarget, m_colorTexture, 0));

	if (depthBuffer)
	{
		T_OGL_SAFE(glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer));
		T_OGL_SAFE(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer));
		m_haveDepth = true;
	}

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	T_OGL_SAFE(glBindFramebuffer(GL_FRAMEBUFFER, 0));

	m_shadowState.wrapS = GL_CLAMP_TO_EDGE;
	m_shadowState.wrapT = GL_CLAMP_TO_EDGE;
	m_shadowState.magFilter = GL_NEAREST;
	m_shadowState.minFilter = GL_NEAREST;

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

ITexture* RenderTargetOpenGLES2::resolve()
{
	return this;
}

int RenderTargetOpenGLES2::getWidth() const
{
	return m_width;
}

int RenderTargetOpenGLES2::getHeight() const
{
	return m_height;
}

bool RenderTargetOpenGLES2::lock(int level, Lock& lock)
{
	return false;
}

void RenderTargetOpenGLES2::unlock(int level)
{
}

void RenderTargetOpenGLES2::bindSampler(GLuint unit, const SamplerStateOpenGL& samplerState, GLint locationTexture)
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
}

void RenderTargetOpenGLES2::bindSize(GLint locationSize)
{
	T_OGL_SAFE(glUniform4f(locationSize, GLfloat(m_width), GLfloat(m_height), GLfloat(1.0f), GLfloat(1.0f)));
}

void RenderTargetOpenGLES2::bind(GLuint primaryDepthTarget)
{
	T_OGL_SAFE(glBindFramebuffer(GL_FRAMEBUFFER, m_frameBufferObject));
	
	if (m_usingPrimaryDepthBuffer)
	{
		T_OGL_SAFE(glFramebufferRenderbuffer(
			GL_FRAMEBUFFER,
			GL_DEPTH_ATTACHMENT,
			GL_RENDERBUFFER,
			primaryDepthTarget
		));
		/*
		T_OGL_SAFE(glFramebufferRenderbuffer(
			GL_FRAMEBUFFER,
			GL_STENCIL_ATTACHMENT,
			GL_RENDERBUFFER_EXT,
			depthBuffer
		));
		*/
	}
	
	glCheckFramebufferStatus(GL_FRAMEBUFFER);
}

void RenderTargetOpenGLES2::enter()
{
	T_OGL_SAFE(glViewport(0, 0, m_width, m_height));
}

	}
}

#endif
