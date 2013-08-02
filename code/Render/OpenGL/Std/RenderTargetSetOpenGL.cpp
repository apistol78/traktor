#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Render/OpenGL/Std/ContextOpenGL.h"
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

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderTargetSetOpenGL", RenderTargetSetOpenGL, RenderTargetSet)

RenderTargetSetOpenGL::RenderTargetSetOpenGL(ContextOpenGL* resourceContext)
:	m_resourceContext(resourceContext)
,	m_targetFBO(0)
,	m_depthBuffer(0)
{
}

RenderTargetSetOpenGL::~RenderTargetSetOpenGL()
{
	destroy();
}

bool RenderTargetSetOpenGL::create(const RenderTargetSetCreateDesc& desc)
{
	T_ASSERT (desc.multiSample <= 1);
	m_desc = desc;

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
			m_desc.width,
			m_desc.height,
			0,
			format,
			type,
			NULL
		));

		m_renderTargets[i] = new RenderTargetOpenGL(
			m_resourceContext,
			m_targetTextures[i],
			m_desc.width,
			m_desc.height
		);
	}

	return true;
}

void RenderTargetSetOpenGL::destroy()
{
	for (uint32_t i = 0; i < sizeof_array(m_renderTargets); ++i)
		safeDestroy(m_renderTargets[i]);

	if (m_targetFBO)
	{
		if (m_resourceContext)
			m_resourceContext->deleteResource(new DeleteFramebufferCallback(m_targetFBO));
		m_targetFBO = 0;
	}
}

int RenderTargetSetOpenGL::getWidth() const
{
	return m_desc.width;
}

int RenderTargetSetOpenGL::getHeight() const
{
	return m_desc.height;
}

ISimpleTexture* RenderTargetSetOpenGL::getColorTexture(int index) const
{
	T_ASSERT (index >= 0);
	T_ASSERT (index < sizeof_array(m_renderTargets));
	return m_renderTargets[index];
}

void RenderTargetSetOpenGL::swap(int index1, int index2)
{
	T_ASSERT (index1 >= 0);
	T_ASSERT (index1 < sizeof_array(m_renderTargets));
	T_ASSERT (index2 >= 0);
	T_ASSERT (index2 < sizeof_array(m_renderTargets));
	std::swap(m_targetTextures[index1], m_targetTextures[index2]);
	std::swap(m_renderTargets[index1], m_renderTargets[index2]);
}

bool RenderTargetSetOpenGL::read(int index, void* buffer) const
{
	return false;
}

bool RenderTargetSetOpenGL::bind(ContextOpenGL* renderContext, GLuint primaryDepthBuffer)
{
	if (!createFramebuffer(primaryDepthBuffer))
		return false;

	T_OGL_SAFE(glBindFramebuffer(GL_FRAMEBUFFER, m_targetFBO));

#if defined(_DEBUG)
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		log::error << L"Framebuffer incomplete!" << Endl;
		return false;
	}
#endif

	const GLenum drawBuffers[] =
	{
		GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3,
		GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5, GL_COLOR_ATTACHMENT6, GL_COLOR_ATTACHMENT7
	};
	T_OGL_SAFE(glDrawBuffers(m_desc.count, drawBuffers));

	T_OGL_SAFE(glViewport(
		0,
		0,
		m_desc.width,
		m_desc.height
	));

	T_OGL_SAFE(glDepthRange(
		0.0f,
		1.0f
	));

	if (m_depthBuffer || m_desc.usingPrimaryDepthStencil)
		renderContext->setPermitDepth(true);
	else
		renderContext->setPermitDepth(false);

	return true;
}

bool RenderTargetSetOpenGL::bind(ContextOpenGL* renderContext, GLuint primaryDepthBuffer, int32_t renderTarget)
{
	if (!createFramebuffer(primaryDepthBuffer))
		return false;

	T_OGL_SAFE(glBindFramebuffer(GL_FRAMEBUFFER, m_targetFBO));

#if defined(_DEBUG)
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		log::error << L"Framebuffer incomplete!" << Endl;
		return false;
	}
#endif
	
	const GLenum drawBuffers[] =
	{
		GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3,
		GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5, GL_COLOR_ATTACHMENT6, GL_COLOR_ATTACHMENT7
	};
	T_OGL_SAFE(glDrawBuffer(drawBuffers[renderTarget]));

	T_OGL_SAFE(glViewport(
		0,
		0,
		m_desc.width,
		m_desc.height
	));

	T_OGL_SAFE(glDepthRange(
		0.0f,
		1.0f
	));

	if (m_depthBuffer || m_desc.usingPrimaryDepthStencil)
		renderContext->setPermitDepth(true);
	else
		renderContext->setPermitDepth(false);

	return true;
}

void RenderTargetSetOpenGL::blit()
{
	T_OGL_SAFE(glBindFramebuffer(GL_READ_FRAMEBUFFER, m_targetFBO));
	T_OGL_SAFE(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0));
	T_OGL_SAFE(glBlitFramebuffer(
		0, 0, m_desc.width, m_desc.height,
		0, m_desc.height, m_desc.width, 0,
		GL_COLOR_BUFFER_BIT,
		GL_NEAREST
	));
}

bool RenderTargetSetOpenGL::createFramebuffer(GLuint primaryDepthBuffer)
{
	// Already created?
	if (m_targetFBO != 0)
		return true;
		
	T_OGL_SAFE(glGenFramebuffers(1, &m_targetFBO));
	T_OGL_SAFE(glBindFramebuffer(GL_FRAMEBUFFER, m_targetFBO));

	// Create depth/stencil buffer.
	if (m_desc.createDepthStencil && !m_desc.usingPrimaryDepthStencil)
	{
		T_OGL_SAFE(glGenRenderbuffers(1, &m_depthBuffer));
		T_OGL_SAFE(glBindRenderbuffer(GL_RENDERBUFFER, m_depthBuffer));

		GLenum format = GL_DEPTH_COMPONENT24;
		if (!m_desc.ignoreStencil)
			format = GL_DEPTH24_STENCIL8;

		T_OGL_SAFE(glRenderbufferStorage(
			GL_RENDERBUFFER,
			format,
			m_desc.width,
			m_desc.height
		));

		T_OGL_SAFE(glFramebufferRenderbuffer(
			GL_FRAMEBUFFER,
			GL_DEPTH_ATTACHMENT,
			GL_RENDERBUFFER,
			m_depthBuffer
		));

		if (!m_desc.ignoreStencil)
		{
			T_OGL_SAFE(glFramebufferRenderbuffer(
				GL_FRAMEBUFFER,
				GL_STENCIL_ATTACHMENT,
				GL_RENDERBUFFER,
				m_depthBuffer
			));
		}
	}

	// Attach primary depth buffer.
	if (m_desc.usingPrimaryDepthStencil)
	{
		T_OGL_SAFE(glFramebufferRenderbuffer(
			GL_FRAMEBUFFER,
			GL_DEPTH_ATTACHMENT,
			GL_RENDERBUFFER,
			primaryDepthBuffer
		));
		if (!m_desc.ignoreStencil)
		{
			T_OGL_SAFE(glFramebufferRenderbuffer(
				GL_FRAMEBUFFER,
				GL_STENCIL_ATTACHMENT,
				GL_RENDERBUFFER,
				primaryDepthBuffer
			));
		}
	}

	// Attach color targets.
	for (int i = 0; i < m_desc.count; ++i)
	{
		T_OGL_SAFE(glFramebufferTexture2D(
			GL_FRAMEBUFFER,
			GL_COLOR_ATTACHMENT0 + i,
			GL_TEXTURE_2D,
			m_targetTextures[i],
			0
		));
	}

	return true;
}

	}
}
