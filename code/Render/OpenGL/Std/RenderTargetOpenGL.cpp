#include "Core/Log/Log.h"
#include "Core/Math/Log2.h"
#include "Render/OpenGL/IContext.h"
#include "Render/OpenGL/Std/BlitHelper.h"
#include "Render/OpenGL/Std/Extensions.h"
#include "Render/OpenGL/Std/RenderTargetOpenGL.h"

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

RenderTargetOpenGL::RenderTargetOpenGL(IContext* resourceContext, BlitHelper* blitHelper)
:	m_resourceContext(resourceContext)
,	m_blitHelper(blitHelper)
,	m_width(0)
,	m_height(0)
,	m_targetWidth(0)
,	m_targetHeight(0)
,	m_textureTarget(0)
,	m_targetFBO(0)
,	m_resolveFBO(0)
,	m_targetColorBuffer(0)
,	m_colorTexture(0)
,	m_haveDepth(false)
,	m_usingPrimaryDepthBuffer(false)
,	m_haveBlitExt(false)
{
}

RenderTargetOpenGL::~RenderTargetOpenGL()
{
	destroy();
}

bool RenderTargetOpenGL::create(const RenderTargetSetCreateDesc& setDesc, const RenderTargetCreateDesc& desc, GLuint depthBuffer, bool backBuffer)
{
	GLenum internalFormat;
	GLint format;
	GLenum type;

	m_width = setDesc.width;
	m_height = setDesc.height;
	m_targetWidth = setDesc.width;
	m_targetHeight = setDesc.height;
	m_textureTarget = GL_TEXTURE_2D;
	m_usingPrimaryDepthBuffer = setDesc.usingPrimaryDepthStencil;

	if (!opengl_have_extension("GL_ARB_texture_non_power_of_two"))
	{
		if (!isLog2(m_width) || !isLog2(m_height))
		{
			m_width = nearestLog2(m_width);
			m_height = nearestLog2(m_height);
		}

		// Only backbuffer allowed to be partially rendered; if expected
		// to be used as texture then we need to ensure everything is renderered.
		if (!backBuffer)
		{
			m_targetWidth = m_width;
			m_targetHeight = m_height;
		}
	}

	switch (desc.format)
	{
	case TfR8:
	case TfR8G8B8A8:
		internalFormat = GL_RGBA;
		format = GL_RGBA;
		type = GL_UNSIGNED_BYTE;
		break;

	case TfR16G16B16A16F:
		if (opengl_have_extension("GL_ARB_texture_float"))
		{
			internalFormat = GL_RGBA16F_ARB;
			format = GL_RGBA;
			type = GL_HALF_FLOAT_ARB;
		}
		else
			return false;
		break;

	case TfR32G32B32A32F:
		if (opengl_have_extension("GL_ARB_texture_float"))
		{
			internalFormat = GL_RGBA32F_ARB;
			format = GL_RGBA;
			type = GL_FLOAT;
		}
		else
			return false;
		break;

#if !defined(__APPLE__)
	case TfR16F:
		if (opengl_have_extension("GL_ARB_texture_float"))
		{
			internalFormat = GL_RGBA16F_ARB;
			format = GL_RED;
			type = GL_HALF_FLOAT_ARB;
		}
		else if (opengl_have_extension("GL_NV_float_buffer"))
		{
			internalFormat = GL_FLOAT_R16_NV;
			format = GL_RED;
			type = GL_FLOAT;
		}
		else if (opengl_have_extension("GL_ATI_texture_float"))
		{
			internalFormat = GL_LUMINANCE_FLOAT16_ATI;
			format = GL_RED;
			type = GL_FLOAT;
		}
		else
			return false;
		break;

	case TfR32F:
		if (opengl_have_extension("GL_ARB_texture_float"))
		{
			internalFormat = GL_RGBA32F_ARB;
			format = GL_RED;
			type = GL_FLOAT;
		}
		else if (opengl_have_extension("GL_NV_float_buffer"))
		{
			internalFormat = GL_FLOAT_R32_NV;
			format = GL_RED;
			type = GL_FLOAT;
		}
		else if (opengl_have_extension("GL_ATI_texture_float"))
		{
			internalFormat = GL_LUMINANCE_FLOAT32_ATI;
			format = GL_RED;
			type = GL_FLOAT;
		}
		else
			return false;
		break;
#else
	case TfR16F:
		if (opengl_have_extension("GL_ARB_texture_float"))
		{
			internalFormat = GL_RGBA16F_ARB;
			format = GL_RED;
			type = GL_HALF_FLOAT_ARB;
		}
		else
			return false;
		break;
			
	case TfR32F:
		if (opengl_have_extension("GL_ARB_texture_float"))
		{
			internalFormat = GL_RGBA32F_ARB;
			format = GL_RED;
			type = GL_FLOAT;
		}
		else
			return false;
		break;
#endif

	default:
		log::error << L"Unable to create render target, unsupported format" << Endl;
		return false;
	}

	if (setDesc.multiSample <= 1)
	{
		T_OGL_SAFE(glGenFramebuffersEXT(1, &m_targetFBO));
		T_OGL_SAFE(glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_targetFBO));

		if (depthBuffer)
		{
			T_OGL_SAFE(glFramebufferRenderbufferEXT(
				GL_FRAMEBUFFER_EXT,
				GL_DEPTH_ATTACHMENT_EXT,
				GL_RENDERBUFFER_EXT,
				depthBuffer
			));
			T_OGL_SAFE(glFramebufferRenderbufferEXT(
				GL_FRAMEBUFFER_EXT,
				GL_STENCIL_ATTACHMENT_EXT,
				GL_RENDERBUFFER_EXT,
				depthBuffer
			));
			m_haveDepth = true;
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

		T_OGL_SAFE(glFramebufferTexture2DEXT(
			GL_FRAMEBUFFER_EXT,
			GL_COLOR_ATTACHMENT0_EXT,
			m_textureTarget,
			m_colorTexture,
			0
		));
	}
	else
	{
		// Multisampled color buffer.
		T_OGL_SAFE(glGenRenderbuffersEXT(1, &m_targetColorBuffer));
		T_OGL_SAFE(glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, m_targetColorBuffer));
		T_OGL_SAFE(glRenderbufferStorageMultisampleEXT(
			GL_RENDERBUFFER_EXT,
			setDesc.multiSample,
			internalFormat,
			m_width,
			m_height
		));
		
		// Create target FBO.
		T_OGL_SAFE(glGenFramebuffersEXT(1, &m_targetFBO));
		T_OGL_SAFE(glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_targetFBO));
		T_OGL_SAFE(glFramebufferRenderbufferEXT(
			GL_FRAMEBUFFER_EXT,
			GL_COLOR_ATTACHMENT0_EXT,
			GL_RENDERBUFFER_EXT,
			m_targetColorBuffer
		));
		if (depthBuffer)
		{
			T_OGL_SAFE(glFramebufferRenderbufferEXT(
				GL_FRAMEBUFFER_EXT,
				GL_DEPTH_ATTACHMENT_EXT,
				GL_RENDERBUFFER_EXT,
				depthBuffer
			));
			T_OGL_SAFE(glFramebufferRenderbufferEXT(
				GL_FRAMEBUFFER_EXT,
				GL_STENCIL_ATTACHMENT_EXT,
				GL_RENDERBUFFER_EXT,
				depthBuffer
			));
			m_haveDepth = true;
		}		
		
		// Create read-back texture.
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
		
		// Create resolve FBO.
		T_OGL_SAFE(glGenFramebuffersEXT(1, &m_resolveFBO));
		T_OGL_SAFE(glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_resolveFBO));
		T_OGL_SAFE(glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, m_colorTexture, 0));
	}

	GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	T_OGL_SAFE(glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0));
	
	m_haveBlitExt = opengl_have_extension("GL_EXT_framebuffer_blit");
	
	std::memset(&m_shadowState, 0, sizeof(m_shadowState));

	return bool(status == GL_FRAMEBUFFER_COMPLETE_EXT);
}

void RenderTargetOpenGL::destroy()
{
	if (m_colorTexture)
	{
		if (m_resourceContext)
			m_resourceContext->deleteResource(new DeleteTextureCallback(m_colorTexture));
		m_colorTexture = 0;
	}
	if (m_resolveFBO)
	{
		if (m_resourceContext)
			m_resourceContext->deleteResource(new DeleteFramebufferCallback(m_resolveFBO));
		m_resolveFBO = 0;
	}
	if (m_targetFBO)
	{
		if (m_resourceContext)
			m_resourceContext->deleteResource(new DeleteFramebufferCallback(m_targetFBO));
		m_targetFBO = 0;
	}
}

int RenderTargetOpenGL::getWidth() const
{
	return m_targetWidth;
}

int RenderTargetOpenGL::getHeight() const
{
	return m_targetHeight;
}

int RenderTargetOpenGL::getDepth() const
{
	return 0;
}

void RenderTargetOpenGL::bind(GLuint unit, const SamplerState& samplerState, GLint locationTexture)
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
	
	T_OGL_SAFE(glUniform1iARB(locationTexture, unit));
}

bool RenderTargetOpenGL::bind(GLuint depthBuffer)
{
	T_OGL_SAFE(glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_targetFBO));

	if (m_usingPrimaryDepthBuffer)
	{
		T_OGL_SAFE(glFramebufferRenderbufferEXT(
			GL_FRAMEBUFFER_EXT,
			GL_DEPTH_ATTACHMENT_EXT,
			GL_RENDERBUFFER_EXT,
			depthBuffer
		));
		T_OGL_SAFE(glFramebufferRenderbufferEXT(
			GL_FRAMEBUFFER_EXT,
			GL_STENCIL_ATTACHMENT_EXT,
			GL_RENDERBUFFER_EXT,
			depthBuffer
		));
	}

	GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	return status == GL_FRAMEBUFFER_COMPLETE_EXT;
}

void RenderTargetOpenGL::enter(GLuint depthBuffer)
{
	T_OGL_SAFE(glViewport(0, 0, m_targetWidth, m_targetHeight));

	if (m_haveDepth || m_usingPrimaryDepthBuffer)
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
}

void RenderTargetOpenGL::resolve()
{
	if (m_resolveFBO)
	{
		T_OGL_SAFE(glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, m_targetFBO));
		T_OGL_SAFE(glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, m_resolveFBO));
		T_OGL_SAFE(glBlitFramebufferEXT(
			0, 0,
			m_targetWidth, m_targetHeight,
			0, 0,
			m_targetWidth, m_targetHeight,
			GL_COLOR_BUFFER_BIT, GL_NEAREST
		));
	}
}

void RenderTargetOpenGL::blit()
{
	if (m_haveBlitExt)
	{
		T_OGL_SAFE(glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, m_targetFBO));
		T_OGL_SAFE(glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, 0));
		T_OGL_SAFE(glBlitFramebufferEXT(
			0, 0, m_targetWidth, m_targetHeight,
			0, m_targetHeight, m_targetWidth, 0,
			GL_COLOR_BUFFER_BIT,
			GL_NEAREST
		));
	}
	else
		m_blitHelper->blit(m_colorTexture);
}

bool RenderTargetOpenGL::read(void* buffer) const
{
	T_OGL_SAFE(glBindTexture(m_textureTarget, m_colorTexture));
	T_OGL_SAFE(glGetTexImage(
		m_textureTarget,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		buffer
	));
	return true;
}

GLuint RenderTargetOpenGL::clearMask() const
{
	if (m_haveDepth)
		return GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT;
	else
		return GL_COLOR_BUFFER_BIT;
}

	}
}
