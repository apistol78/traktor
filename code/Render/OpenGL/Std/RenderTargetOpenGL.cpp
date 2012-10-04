#include <cstring>
#include "Core/Log/Log.h"
#include "Core/Math/Log2.h"
#include "Render/OpenGL/Std/BlitHelper.h"
#include "Render/OpenGL/Std/ContextOpenGL.h"
#include "Render/OpenGL/Std/Extensions.h"
#include "Render/OpenGL/Std/RenderTargetOpenGL.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderTargetOpenGL", RenderTargetOpenGL, ISimpleTexture)

RenderTargetOpenGL::RenderTargetOpenGL(GLuint colorTexture, int32_t width, int32_t height)
:	m_colorTexture(colorTexture)
,	m_width(width)
,	m_height(height)
{
}

RenderTargetOpenGL::~RenderTargetOpenGL()
{
	destroy();
}

void RenderTargetOpenGL::destroy()
{
}

ITexture* RenderTargetOpenGL::resolve()
{
	return this;
}

int RenderTargetOpenGL::getWidth() const
{
	return m_width;
}

int RenderTargetOpenGL::getHeight() const
{
	return m_height;
}

bool RenderTargetOpenGL::lock(int level, Lock& lock)
{
	return false;
}

void RenderTargetOpenGL::unlock(int level)
{
}

void RenderTargetOpenGL::bindSampler(GLuint unit, const SamplerState& samplerState, GLint locationTexture)
{
	T_OGL_SAFE(glActiveTexture(GL_TEXTURE0 + unit));
	T_OGL_SAFE(glBindTexture(GL_TEXTURE_2D, m_colorTexture));

	//if (!opengl_have_extension(E_T_rendertarget_nearest_filter_only))
	//{
	//	GLenum minFilter = GL_NEAREST;
	//	if (samplerState.minFilter != GL_NEAREST)
	//		minFilter = GL_LINEAR;
	//	else
	//		minFilter = GL_NEAREST;

	//	if (m_shadowState.minFilter != minFilter)
	//	{
	//		T_OGL_SAFE(glTexParameteri(m_textureTarget, GL_TEXTURE_MIN_FILTER, minFilter));
	//		m_shadowState.minFilter = minFilter;
	//	}

	//	if (m_shadowState.magFilter != samplerState.magFilter)
	//	{
	//		T_OGL_SAFE(glTexParameteri(m_textureTarget, GL_TEXTURE_MAG_FILTER, samplerState.magFilter));
	//		m_shadowState.magFilter = samplerState.magFilter;
	//	}

	//	if (m_shadowState.wrapS != samplerState.wrapS)
	//	{
	//		T_OGL_SAFE(glTexParameteri(m_textureTarget, GL_TEXTURE_WRAP_S, samplerState.wrapS));
	//		m_shadowState.wrapS = samplerState.wrapS;
	//	}

	//	if (m_shadowState.wrapT != samplerState.wrapT)
	//	{
	//		T_OGL_SAFE(glTexParameteri(m_textureTarget, GL_TEXTURE_WRAP_T, samplerState.wrapT));
	//		m_shadowState.wrapT = samplerState.wrapT;
	//	}
	//}

	T_OGL_SAFE(glUniform1iARB(locationTexture, unit));
}

void RenderTargetOpenGL::bindSize(GLint locationSize)
{
	T_OGL_SAFE(glUniform4fARB(locationSize, GLfloat(m_width), GLfloat(m_height), GLfloat(1.0f), GLfloat(1.0f)));
}

//bool RenderTargetOpenGL::bind(ContextOpenGL* renderContext, GLuint depthBuffer)
//{
//	T_OGL_SAFE(glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_targetFBO));
//
//	if (m_usingPrimaryDepthBuffer)
//	{
//		T_OGL_SAFE(glFramebufferRenderbufferEXT(
//			GL_FRAMEBUFFER_EXT,
//			GL_DEPTH_ATTACHMENT_EXT,
//			GL_RENDERBUFFER_EXT,
//			depthBuffer
//		));
//		T_OGL_SAFE(glFramebufferRenderbufferEXT(
//			GL_FRAMEBUFFER_EXT,
//			GL_STENCIL_ATTACHMENT_EXT,
//			GL_RENDERBUFFER_EXT,
//			depthBuffer
//		));
//	}
//
//	GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
//	if (status != GL_FRAMEBUFFER_COMPLETE_EXT)
//	{
//		log::error << L"Unable to bind render target; framebuffer not complete" << Endl;
//		return false;
//	}
//
//	T_OGL_SAFE(glViewport(
//		0,
//		0,
//		m_width,
//		m_height
//	));
//
//	T_OGL_SAFE(glDepthRange(
//		0.0f,
//		1.0f
//	));
//
//	if (m_haveDepth || m_usingPrimaryDepthBuffer)
//		renderContext->setPermitDepth(true);
//	else
//		renderContext->setPermitDepth(false);
//
//	return true;
//}
//
//void RenderTargetOpenGL::enter()
//{
//	T_OGL_SAFE(glViewport(0, 0, m_targetWidth, m_targetHeight));
//}
//
//void RenderTargetOpenGL::resolveTarget()
//{
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
//}
//
//void RenderTargetOpenGL::blit()
//{
//	if (m_haveBlitExt)
//	{
//		T_OGL_SAFE(glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, m_targetFBO));
//		T_OGL_SAFE(glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, 0));
//		T_OGL_SAFE(glBlitFramebufferEXT(
//			0, 0, m_targetWidth, m_targetHeight,
//			0, m_targetHeight, m_targetWidth, 0,
//			GL_COLOR_BUFFER_BIT,
//			GL_NEAREST
//		));
//	}
//	else
//		m_blitHelper->blit(m_colorTexture);
//}
//
//bool RenderTargetOpenGL::read(void* buffer) const
//{
//	T_OGL_SAFE(glBindTexture(m_textureTarget, m_colorTexture));
//	T_OGL_SAFE(glGetTexImage(
//		m_textureTarget,
//		0,
//		GL_RGBA,
//		GL_UNSIGNED_BYTE,
//		buffer
//	));
//	return true;
//}

	}
}
