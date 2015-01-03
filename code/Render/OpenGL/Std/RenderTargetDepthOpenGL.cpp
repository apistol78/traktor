#include "Render/OpenGL/Std/ContextOpenGL.h"
#include "Render/OpenGL/Std/RenderTargetDepthOpenGL.h"

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

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderTargetDepthOpenGL", RenderTargetDepthOpenGL, ISimpleTexture)

RenderTargetDepthOpenGL::RenderTargetDepthOpenGL(ContextOpenGL* resourceContext, GLuint depthTexture, int32_t width, int32_t height)
:	m_resourceContext(resourceContext)
,	m_depthTexture(depthTexture)
,	m_width(width)
,	m_height(height)
{
}

RenderTargetDepthOpenGL::~RenderTargetDepthOpenGL()
{
	destroy();
}

void RenderTargetDepthOpenGL::destroy()
{
	if (m_depthTexture)
	{
		if (m_resourceContext)
			m_resourceContext->deleteResource(new DeleteTextureCallback(m_depthTexture));
		m_depthTexture = 0;
	}
}

ITexture* RenderTargetDepthOpenGL::resolve()
{
	return this;
}

int RenderTargetDepthOpenGL::getWidth() const
{
	return m_width;
}

int RenderTargetDepthOpenGL::getHeight() const
{
	return m_height;
}

bool RenderTargetDepthOpenGL::lock(int level, Lock& lock)
{
	return false;
}

void RenderTargetDepthOpenGL::unlock(int level)
{
}

void RenderTargetDepthOpenGL::bindTexture(ContextOpenGL* renderContext, uint32_t samplerObject)
{
	T_OGL_SAFE(glBindTexture(GL_TEXTURE_2D, m_depthTexture));
	renderContext->bindSamplerStateObject(GL_TEXTURE_2D, samplerObject, false, 0.0f);
}

void RenderTargetDepthOpenGL::bindSize(GLint locationSize)
{
	T_OGL_SAFE(glUniform4f(locationSize, GLfloat(m_width), GLfloat(m_height), GLfloat(1.0f), GLfloat(1.0f)));
}

	}
}
