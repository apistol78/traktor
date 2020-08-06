#include "Core/Log/Log.h"
#include "Core/Math/Log2.h"
#include "Core/Misc/SafeDestroy.h"
#include "Render/OpenGL/Std/RenderContextOpenGL.h"
#include "Render/OpenGL/Std/RenderTargetDepthOpenGL.h"
#include "Render/OpenGL/Std/RenderTargetOpenGL.h"
#include "Render/OpenGL/Std/RenderTargetSetOpenGL.h"
#include "Render/OpenGL/Std/ResourceContextOpenGL.h"
#include "Render/OpenGL/Std/UtilitiesOpenGL.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

struct DeleteRenderBufferCallback : public ResourceContextOpenGL::IDeleteCallback
{
	GLuint m_renderBufferName;

	DeleteRenderBufferCallback(GLuint renderBufferName)
	:	m_renderBufferName(renderBufferName)
	{
	}

	virtual void deleteResource()
	{
		T_OGL_SAFE(glDeleteRenderbuffers(1, &m_renderBufferName));
		T_OGL_SAFE(glFinish());
		delete this;
	}
};

struct DeleteFramebufferCallback : public ResourceContextOpenGL::IDeleteCallback
{
	GLuint m_framebufferName;

	DeleteFramebufferCallback(GLuint framebufferName)
	:	m_framebufferName(framebufferName)
	{
	}

	virtual void deleteResource()
	{
		T_OGL_SAFE(glDeleteFramebuffers(1, &m_framebufferName));
		T_OGL_SAFE(glFinish());
		delete this;
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderTargetSetOpenGL", RenderTargetSetOpenGL, IRenderTargetSet)

uint32_t RenderTargetSetOpenGL::ms_primaryTargetTag = 1;

RenderTargetSetOpenGL::RenderTargetSetOpenGL(ResourceContextOpenGL* resourceContext)
:	m_resourceContext(resourceContext)
,	m_targetFBO(0)
,	m_depthBufferOrTexture(0)
,	m_currentTag(0)
,	m_depthTargetShared(false)
,	m_contentValid(false)
{
}

RenderTargetSetOpenGL::~RenderTargetSetOpenGL()
{
	destroy();
}

bool RenderTargetSetOpenGL::create(const RenderTargetSetCreateDesc& desc, IRenderTargetSet* sharedDepthStencil)
{
	T_ASSERT(desc.multiSample <= 1);
	m_desc = desc;

	// Create depth/stencil buffer.
	if (m_desc.createDepthStencil)
	{
		GLenum format = GL_DEPTH_COMPONENT32; // GL_DEPTH_COMPONENT24;
		if (!m_desc.ignoreStencil)
			format = GL_DEPTH24_STENCIL8;

		if (!desc.usingDepthStencilAsTexture)
		{
			T_OGL_SAFE(glGenRenderbuffers(1, &m_depthBufferOrTexture));
			T_OGL_SAFE(glBindRenderbuffer(GL_RENDERBUFFER, m_depthBufferOrTexture));

			T_OGL_SAFE(glRenderbufferStorage(
				GL_RENDERBUFFER,
				format,
				desc.width,
				desc.height
			));
		}
		else
		{
			T_OGL_SAFE(glGenTextures(1, &m_depthBufferOrTexture));
			T_OGL_SAFE(glBindTexture(GL_TEXTURE_2D, m_depthBufferOrTexture));

			T_OGL_SAFE(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
			T_OGL_SAFE(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
			T_OGL_SAFE(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
			T_OGL_SAFE(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));

			T_OGL_SAFE(glTexImage2D(
				GL_TEXTURE_2D,
				0,
				format,
				m_desc.width,
				m_desc.height,
				0,
				GL_DEPTH_COMPONENT,
				GL_FLOAT,
				NULL
			));
		}

		m_depthTarget = new RenderTargetDepthOpenGL(
			m_resourceContext,
			m_depthBufferOrTexture,
			m_desc.width,
			m_desc.height
		);
		m_depthTargetShared = false;
	}
	else if (sharedDepthStencil)
	{
		m_depthTarget = dynamic_type_cast< RenderTargetDepthOpenGL* >(sharedDepthStencil->getDepthTexture());
		m_depthTargetShared = true;
	}

	// Create color targets.
	T_ASSERT(desc.count < sizeof_array(m_targetTextures));
	T_OGL_SAFE(glGenTextures(desc.count, m_targetTextures));

	for (int32_t i = 0; i < desc.count; ++i)
	{
		GLenum internalFormat;
		GLint format;
		GLenum type;

		if (!convertTargetFormat(desc.targets[i].format, internalFormat, format, type))
			return false;

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

		int32_t mips = 1;
		if (desc.generateMips)
		{
			T_OGL_SAFE(glGenerateMipmap(GL_TEXTURE_2D));
			mips = 1 + log2(std::max(m_desc.width, m_desc.height));
		}

		m_colorTargets[i] = new RenderTargetOpenGL(
			m_resourceContext,
			m_targetTextures[i],
			m_desc.width,
			m_desc.height,
			mips
		);
	}

	return true;
}

void RenderTargetSetOpenGL::destroy()
{
	for (uint32_t i = 0; i < sizeof_array(m_colorTargets); ++i)
		safeDestroy(m_colorTargets[i]);

	if (m_depthBufferOrTexture)
	{
		if (!m_depthTargetShared)
		{
			if (m_resourceContext)
				m_resourceContext->deleteResource(new DeleteRenderBufferCallback(m_depthBufferOrTexture));
		}
		m_depthBufferOrTexture = 0;
	}

	if (m_targetFBO)
	{
		if (m_resourceContext)
			m_resourceContext->deleteResource(new DeleteFramebufferCallback(m_targetFBO));
		m_targetFBO = 0;
	}
}

int32_t RenderTargetSetOpenGL::getWidth() const
{
	return m_desc.width;
}

int32_t RenderTargetSetOpenGL::getHeight() const
{
	return m_desc.height;
}

ISimpleTexture* RenderTargetSetOpenGL::getColorTexture(int32_t index) const
{
	T_ASSERT(index >= 0);
	T_ASSERT(index < sizeof_array(m_colorTargets));
	return m_colorTargets[index];
}

ISimpleTexture* RenderTargetSetOpenGL::getDepthTexture() const
{
	return m_depthTarget;
}

bool RenderTargetSetOpenGL::isContentValid() const
{
	return m_contentValid;
}

bool RenderTargetSetOpenGL::read(int32_t index, void* buffer) const
{
#if !defined(__APPLE__)
	GLenum internalFormat;
	GLint format;
	GLenum type;

	if (!convertTargetFormat(m_desc.targets[index].format, internalFormat, format, type))
		return false;

	T_OGL_SAFE(glBindFramebuffer(GL_FRAMEBUFFER, m_targetFBO));
	T_OGL_SAFE(glReadBuffer((GLenum)GL_COLOR_ATTACHMENT0_EXT + index));
	T_OGL_SAFE(glReadPixels(0, 0, m_desc.width, m_desc.height, GL_RGBA, GL_FLOAT, (GLvoid*)buffer));
	return true;
#else
	return false;
#endif
}

bool RenderTargetSetOpenGL::bind(RenderContextOpenGL* renderContext, GLuint primaryDepthBuffer, bool havePrimaryStencil)
{
	if (!createFramebuffer(primaryDepthBuffer, havePrimaryStencil))
		return false;

	T_OGL_SAFE(glBindFramebuffer(GL_FRAMEBUFFER, m_targetFBO));

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

	if (m_depthBufferOrTexture || m_desc.usingPrimaryDepthStencil)
		renderContext->setPermitDepth(true);
	else
		renderContext->setPermitDepth(false);

	return true;
}

bool RenderTargetSetOpenGL::bind(RenderContextOpenGL* renderContext, GLuint primaryDepthBuffer, bool havePrimaryStencil, int32_t renderTarget)
{
	if (!createFramebuffer(primaryDepthBuffer, havePrimaryStencil))
		return false;

	T_OGL_SAFE(glBindFramebuffer(GL_FRAMEBUFFER, m_targetFBO));

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

	if (m_depthBufferOrTexture || m_desc.usingPrimaryDepthStencil)
		renderContext->setPermitDepth(true);
	else
		renderContext->setPermitDepth(false);

	return true;
}

void RenderTargetSetOpenGL::unbind()
{
#if !defined(__APPLE__)
	if (m_desc.generateMips)
	{
		for (int32_t i = 0; i < m_desc.count; ++i)
			T_OGL_SAFE(glGenerateTextureMipmap(m_targetTextures[i]));
	}
#endif
}

void RenderTargetSetOpenGL::blit(RenderContextOpenGL* renderContext)
{
	int32_t physicalWidth = renderContext->getPhysicalWidth();
	int32_t physicalHeight = renderContext->getPhysicalHeight();

	T_OGL_SAFE(glBindFramebuffer(GL_READ_FRAMEBUFFER, m_targetFBO));
	T_OGL_SAFE(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0));

	T_OGL_SAFE(glBlitFramebuffer(
		0, 0, m_desc.width, m_desc.height,
		0, physicalHeight, physicalWidth, 0,
		GL_COLOR_BUFFER_BIT,
		(physicalWidth == m_desc.width && physicalHeight == m_desc.height) ? GL_NEAREST : GL_LINEAR
	));
}

bool RenderTargetSetOpenGL::createFramebuffer(GLuint primaryDepthBuffer, bool havePrimaryStencil)
{
	// Already created?
	if (m_targetFBO != 0 && (!m_desc.usingPrimaryDepthStencil || m_currentTag == ms_primaryTargetTag))
		return true;

	// If re-creating then we need to make sure we don't leak FBOs.
	if (m_targetFBO)
	{
		if (m_resourceContext)
			m_resourceContext->deleteResource(new DeleteFramebufferCallback(m_targetFBO));
	}

	T_OGL_SAFE(glGenFramebuffers(1, &m_targetFBO));
	T_OGL_SAFE(glBindFramebuffer(GL_FRAMEBUFFER, m_targetFBO));

	// Attach depth buffer.
	if (m_desc.createDepthStencil && !m_desc.usingPrimaryDepthStencil)
	{
		if (!m_desc.usingDepthStencilAsTexture)
		{
			T_OGL_SAFE(glFramebufferRenderbuffer(
				GL_FRAMEBUFFER,
				GL_DEPTH_ATTACHMENT,
				GL_RENDERBUFFER,
				m_depthBufferOrTexture
			));

			if (!m_desc.ignoreStencil)
			{
				T_OGL_SAFE(glFramebufferRenderbuffer(
					GL_FRAMEBUFFER,
					GL_STENCIL_ATTACHMENT,
					GL_RENDERBUFFER,
					m_depthBufferOrTexture
				));
			}
		}
		else
		{
			T_OGL_SAFE(glFramebufferTexture2D(
				GL_FRAMEBUFFER,
				GL_DEPTH_ATTACHMENT,
				GL_TEXTURE_2D,
				m_depthBufferOrTexture,
				0
			));
		}
	}
	else if (m_desc.usingPrimaryDepthStencil)
	{
		T_OGL_SAFE(glFramebufferRenderbuffer(
			GL_FRAMEBUFFER,
			GL_DEPTH_ATTACHMENT,
			GL_RENDERBUFFER,
			primaryDepthBuffer
		));
		if (!m_desc.ignoreStencil && havePrimaryStencil)
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

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		log::error << L"Unexpected renderer error; incomplete framebuffer object (" << getOpenGLErrorString(status) << L")!" << Endl;
		return false;
	}

	m_currentTag = ms_primaryTargetTag;
	return true;
}

	}
}
