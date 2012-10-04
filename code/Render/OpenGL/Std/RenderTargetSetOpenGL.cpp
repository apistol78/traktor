#include "Core/Log/Log.h"
//#include "Core/Math/Log2.h"
#include "Core/Misc/SafeDestroy.h"
#include "Render/OpenGL/Std/BlitHelper.h"
#include "Render/OpenGL/Std/ContextOpenGL.h"
#include "Render/OpenGL/Std/Extensions.h"
#include "Render/OpenGL/Std/RenderTargetOpenGL.h"
#include "Render/OpenGL/Std/RenderTargetSetOpenGL.h"
#include "Render/OpenGL/Std/UtilitiesOpenGL.h"

namespace traktor
{
	namespace render
	{
		namespace
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

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderTargetSetOpenGL", RenderTargetSetOpenGL, RenderTargetSet)

RenderTargetSetOpenGL::RenderTargetSetOpenGL(ContextOpenGL* resourceContext, BlitHelper* blitHelper)
:	m_resourceContext(resourceContext)
,	m_blitHelper(blitHelper)
,	m_haveBlitExt(false)
,	m_width(0)
,	m_height(0)
,	m_targetFBO(0)
,	m_depthBuffer(0)
,	m_usingPrimaryDepth(false)
,	m_targetCount(0)
{
	m_haveBlitExt = opengl_have_extension(E_GL_EXT_framebuffer_blit);
}

RenderTargetSetOpenGL::~RenderTargetSetOpenGL()
{
	destroy();
}

bool RenderTargetSetOpenGL::create(const RenderTargetSetCreateDesc& desc)
{
	if (!opengl_have_extension(E_GL_EXT_framebuffer_object))
	{
		log::error << L"Cannot create FBO; not supported by OpenGL driver" << Endl;
		return false;
	}

	// When using MSAA we require both MSAA framebuffer extension and framebuffer blitting.
	if (
		desc.multiSample > 1 &&
		!(opengl_have_extension(E_GL_EXT_framebuffer_multisample) && opengl_have_extension(E_GL_EXT_framebuffer_blit))
	)
	{
		log::error << L"Cannot create multisample FBO; not supported by OpenGL driver" << Endl;
		return false;
	}

	m_width = desc.width;
	m_height = desc.height;

	//m_targetWidth = desc.width;
	//m_targetHeight = desc.height;

	//if (!opengl_have_extension(E_T_rendertarget_non_power_of_two))
	//{
	//	if (!isLog2(m_width) || !isLog2(m_height))
	//	{
	//		log::warning << L"Trying to create non-power-of-2 render target but OpenGL doesn't support NPO2; scaling applied" << Endl;
	//		m_width = nearestLog2(m_width);
	//		m_height = nearestLog2(m_height);
	//	}
	//	
	//	// Only backbuffer allowed to be partially rendered; if expected
	//	// to be used as texture then we need to ensure everything is renderered.
	//	if (!backBuffer)
	//	{
	//		m_targetWidth = m_width;
	//		m_targetHeight = m_height;
	//	}
	//}

	T_OGL_SAFE(glGenFramebuffersEXT(1, &m_targetFBO));
	T_OGL_SAFE(glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_targetFBO));

	// Create depth/stencil buffer.
	if (desc.createDepthStencil)
	{
		T_OGL_SAFE(glGenRenderbuffersEXT(1, &m_depthBuffer));
		T_OGL_SAFE(glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, m_depthBuffer));

#if !defined(__APPLE__)
		GLenum format = GL_DEPTH_COMPONENT24;
		if (!desc.ignoreStencil)
			format = GL_DEPTH24_STENCIL8_EXT;
#else
		GLenum format = GL_DEPTH_STENCIL_EXT;
#endif
		
		if (desc.multiSample <= 1)
		{
			T_OGL_SAFE(glRenderbufferStorageEXT(
				GL_RENDERBUFFER_EXT,
				format,
				m_width,
				m_height
			));
		}
		else
		{
			T_OGL_SAFE(glRenderbufferStorageMultisampleEXT(
				GL_RENDERBUFFER_EXT,
				desc.multiSample,
				format,
				m_width,
				m_height
			));
		}

		T_OGL_SAFE(glFramebufferRenderbufferEXT(
			GL_FRAMEBUFFER_EXT,
			GL_DEPTH_ATTACHMENT_EXT,
			GL_RENDERBUFFER_EXT,
			m_depthBuffer
		));
		T_OGL_SAFE(glFramebufferRenderbufferEXT(
			GL_FRAMEBUFFER_EXT,
			GL_STENCIL_ATTACHMENT_EXT,
			GL_RENDERBUFFER_EXT,
			m_depthBuffer
		));
	}

	// Create color targets.
	T_ASSERT (desc.count < sizeof_array(m_targetTextures));
	T_OGL_SAFE(glGenTextures(desc.count, m_targetTextures));

	for (int i = 0; i < desc.count; ++i)
	{
		GLenum internalFormat;
		GLint format;
		GLenum type;

		if (!convertTargetFormat(desc.targets[i].format, internalFormat, format, type))
			return false;

		if (desc.multiSample <= 1)
		{
			T_OGL_SAFE(glActiveTexture(GL_TEXTURE0));
			T_OGL_SAFE(glBindTexture(GL_TEXTURE_2D, m_targetTextures[i]));

			T_OGL_SAFE(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
			T_OGL_SAFE(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
			T_OGL_SAFE(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
			T_OGL_SAFE(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));

			T_OGL_SAFE(glTexImage2D(
				GL_TEXTURE_2D,
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
				GL_COLOR_ATTACHMENT0_EXT + i,
				GL_TEXTURE_2D,
				m_targetTextures[i],
				0
			));
		}
		else
		{
			//// Multisampled color buffer.
			//T_OGL_SAFE(glGenRenderbuffersEXT(1, &m_targetColorBuffer));
			//T_OGL_SAFE(glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, m_targetColorBuffer));
			//T_OGL_SAFE(glRenderbufferStorageMultisampleEXT(
			//	GL_RENDERBUFFER_EXT,
			//	setDesc.multiSample,
			//	internalFormat,
			//	m_width,
			//	m_height
			//));

			//// Create target FBO.
			//T_OGL_SAFE(glGenFramebuffersEXT(1, &m_targetFBO));
			//T_OGL_SAFE(glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_targetFBO));
			//T_OGL_SAFE(glFramebufferRenderbufferEXT(
			//	GL_FRAMEBUFFER_EXT,
			//	GL_COLOR_ATTACHMENT0_EXT,
			//	GL_RENDERBUFFER_EXT,
			//	m_targetColorBuffer
			//));
			//if (depthBuffer)
			//{
			//	T_OGL_SAFE(glFramebufferRenderbufferEXT(
			//		GL_FRAMEBUFFER_EXT,
			//		GL_DEPTH_ATTACHMENT_EXT,
			//		GL_RENDERBUFFER_EXT,
			//		depthBuffer
			//	));
			//	T_OGL_SAFE(glFramebufferRenderbufferEXT(
			//		GL_FRAMEBUFFER_EXT,
			//		GL_STENCIL_ATTACHMENT_EXT,
			//		GL_RENDERBUFFER_EXT,
			//		depthBuffer
			//	));
			//	m_haveDepth = true;
			//}

			//// Create read-back texture.
			//T_OGL_SAFE(glGenTextures(1, &m_colorTexture));

			//T_OGL_SAFE(glActiveTexture(GL_TEXTURE0));
			//T_OGL_SAFE(glBindTexture(m_textureTarget, m_colorTexture));

			//T_OGL_SAFE(glTexParameterf(m_textureTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
			//T_OGL_SAFE(glTexParameterf(m_textureTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
			//T_OGL_SAFE(glTexParameterf(m_textureTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
			//T_OGL_SAFE(glTexParameterf(m_textureTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST));

			//T_OGL_SAFE(glTexImage2D(
			//	m_textureTarget,
			//	0,
			//	internalFormat,
			//	m_width,
			//	m_height,
			//	0,
			//	format,
			//	type,
			//	NULL
			//));

			//// Create resolve FBO.
			//T_OGL_SAFE(glGenFramebuffersEXT(1, &m_resolveFBO));
			//T_OGL_SAFE(glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_resolveFBO));
			//T_OGL_SAFE(glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, m_colorTexture, 0));
		}

		m_renderTargets[i] = new RenderTargetOpenGL(
			m_targetTextures[i],
			m_width,
			m_height
		);
	}

	m_usingPrimaryDepth = desc.usingPrimaryDepthStencil;
	m_targetCount = desc.count;

	return true;
}

void RenderTargetSetOpenGL::destroy()
{
	for (uint32_t i = 0; i < sizeof_array(m_renderTargets); ++i)
		safeDestroy(m_renderTargets[i]);

	//if (m_resolveFBO)
	//{
	//	if (m_resourceContext)
	//		m_resourceContext->deleteResource(new DeleteFramebufferCallback(m_resolveFBO));
	//	m_resolveFBO = 0;
	//}
	if (m_targetFBO)
	{
		if (m_resourceContext)
			m_resourceContext->deleteResource(new DeleteFramebufferCallback(m_targetFBO));
		m_targetFBO = 0;
	}
}

int RenderTargetSetOpenGL::getWidth() const
{
	return m_width;
}

int RenderTargetSetOpenGL::getHeight() const
{
	return m_height;
}

ISimpleTexture* RenderTargetSetOpenGL::getColorTexture(int index) const
{
	T_ASSERT (index >= 0);
	T_ASSERT (index < 8);
	return m_renderTargets[index];
}

void RenderTargetSetOpenGL::swap(int index1, int index2)
{
	std::swap(m_targetTextures[index1], m_targetTextures[index2]);
	std::swap(m_renderTargets[index1], m_renderTargets[index2]);
}

bool RenderTargetSetOpenGL::read(int index, void* buffer) const
{
	return false;
}

bool RenderTargetSetOpenGL::bind(ContextOpenGL* renderContext, GLuint primaryDepthBuffer)
{
	T_OGL_SAFE(glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_targetFBO));

	if (m_usingPrimaryDepth)
	{
		T_OGL_SAFE(glFramebufferRenderbufferEXT(
			GL_FRAMEBUFFER_EXT,
			GL_DEPTH_ATTACHMENT_EXT,
			GL_RENDERBUFFER_EXT,
			primaryDepthBuffer
		));
		T_OGL_SAFE(glFramebufferRenderbufferEXT(
			GL_FRAMEBUFFER_EXT,
			GL_STENCIL_ATTACHMENT_EXT,
			GL_RENDERBUFFER_EXT,
			primaryDepthBuffer
		));
	}

	GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT)
		return false;

	const GLenum drawBuffers[] =
	{
		GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_COLOR_ATTACHMENT2_EXT, GL_COLOR_ATTACHMENT3_EXT,
		GL_COLOR_ATTACHMENT4_EXT, GL_COLOR_ATTACHMENT5_EXT, GL_COLOR_ATTACHMENT6_EXT, GL_COLOR_ATTACHMENT7_EXT
	};
	T_OGL_SAFE(glDrawBuffersARB(m_targetCount, drawBuffers));

	T_OGL_SAFE(glViewport(
		0,
		0,
		m_width,
		m_height
	));

	T_OGL_SAFE(glDepthRange(
		0.0f,
		1.0f
	));

	if (m_depthBuffer || m_usingPrimaryDepth)
		renderContext->setPermitDepth(true);
	else
		renderContext->setPermitDepth(false);

	return true;
}

bool RenderTargetSetOpenGL::bind(ContextOpenGL* renderContext, GLuint primaryDepthBuffer, int32_t renderTarget)
{
	T_OGL_SAFE(glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_targetFBO));

	if (m_usingPrimaryDepth)
	{
		T_OGL_SAFE(glFramebufferRenderbufferEXT(
			GL_FRAMEBUFFER_EXT,
			GL_DEPTH_ATTACHMENT_EXT,
			GL_RENDERBUFFER_EXT,
			primaryDepthBuffer
		));
		T_OGL_SAFE(glFramebufferRenderbufferEXT(
			GL_FRAMEBUFFER_EXT,
			GL_STENCIL_ATTACHMENT_EXT,
			GL_RENDERBUFFER_EXT,
			primaryDepthBuffer
		));
	}

	GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT)
		return false;

	T_OGL_SAFE(glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT + renderTarget));

	T_OGL_SAFE(glViewport(
		0,
		0,
		m_width,
		m_height
	));

	T_OGL_SAFE(glDepthRange(
		0.0f,
		1.0f
	));

	if (m_depthBuffer || m_usingPrimaryDepth)
		renderContext->setPermitDepth(true);
	else
		renderContext->setPermitDepth(false);

	return true;
}

void RenderTargetSetOpenGL::resolve()
{
//	if (m_resolveFBO)
//	{
//		T_OGL_SAFE(glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, m_targetFBO));
//		T_OGL_SAFE(glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, m_resolveFBO));
//		T_OGL_SAFE(glBlitFramebufferEXT(
//			0, 0,
//			m_targetWidth, m_targetHeight,
//			0, 0,
//			m_targetWidth, m_targetHeight,
//			GL_COLOR_BUFFER_BIT, GL_NEAREST
//		));
//	}
}

void RenderTargetSetOpenGL::blit()
{
	if (m_haveBlitExt)
	{
		T_OGL_SAFE(glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, m_targetFBO));
		T_OGL_SAFE(glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, 0));
		T_OGL_SAFE(glBlitFramebufferEXT(
			0, 0, m_width, m_height,
			0, m_height, m_width, 0,
			GL_COLOR_BUFFER_BIT,
			GL_NEAREST
		));
	}
	else
		m_blitHelper->blit(m_targetTextures[0]);
}

	}
}
