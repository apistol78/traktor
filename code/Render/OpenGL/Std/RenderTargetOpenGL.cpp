#include "Render/OpenGL/Std/ContextOpenGL.h"
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

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderTargetOpenGL", RenderTargetOpenGL, ISimpleTexture)

RenderTargetOpenGL::RenderTargetOpenGL(ContextOpenGL* resourceContext, GLuint colorTexture, int32_t width, int32_t height)
:	m_resourceContext(resourceContext)
,	m_colorTexture(colorTexture)
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
	if (m_colorTexture)
	{
		//if (m_resourceContext)
		//	m_resourceContext->deleteResource(new DeleteTextureCallback(m_colorTexture));
		m_colorTexture = 0;
	}
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

void RenderTargetOpenGL::bindSampler(ContextOpenGL* renderContext, GLuint unit, const GLuint sampler[], GLint locationTexture)
{
	T_OGL_SAFE(glActiveTexture(GL_TEXTURE0 + unit));
	T_OGL_SAFE(glBindTexture(GL_TEXTURE_2D, m_colorTexture));
	T_OGL_SAFE(glBindSampler(unit, sampler[0]));
	T_OGL_SAFE(glUniform1iARB(locationTexture, unit));
}

void RenderTargetOpenGL::bindSize(GLint locationSize)
{
	T_OGL_SAFE(glUniform4fARB(locationSize, GLfloat(m_width), GLfloat(m_height), GLfloat(1.0f), GLfloat(1.0f)));
}

	}
}
