#include "Render/OpenGL/Std/RenderContextOpenGL.h"
#include "Render/OpenGL/Std/RenderTargetOpenGL.h"
#include "Render/OpenGL/Std/ResourceContextOpenGL.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

struct DeleteTextureCallback : public ResourceContextOpenGL::IDeleteCallback
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

RenderTargetOpenGL::RenderTargetOpenGL(ResourceContextOpenGL* resourceContext, GLuint colorTexture, int32_t width, int32_t height)
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
		if (m_resourceContext)
			m_resourceContext->deleteResource(new DeleteTextureCallback(m_colorTexture));
		m_colorTexture = 0;
	}
}

ITexture* RenderTargetOpenGL::resolve()
{
	return this;
}

int32_t RenderTargetOpenGL::getWidth() const
{
	return m_width;
}

int32_t RenderTargetOpenGL::getHeight() const
{
	return m_height;
}

int32_t RenderTargetOpenGL::getMips() const
{
	return 1;
}

bool RenderTargetOpenGL::lock(int32_t level, Lock& lock)
{
	return false;
}

void RenderTargetOpenGL::unlock(int32_t level)
{
}

void* RenderTargetOpenGL::getInternalHandle()
{
	return (void*)m_colorTexture;
}

void RenderTargetOpenGL::bindTexture(GLuint textureUnit) const
{
	T_OGL_SAFE(glActiveTexture(GL_TEXTURE0 + textureUnit));
	T_OGL_SAFE(glBindTexture(GL_TEXTURE_2D, m_colorTexture));
}

void RenderTargetOpenGL::bindImage(GLuint imageUnit) const
{
	T_OGL_SAFE(glBindImageTexture(imageUnit, m_colorTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F));
}

void RenderTargetOpenGL::bindSize(GLint locationSize) const
{
	T_OGL_SAFE(glUniform4f(locationSize, GLfloat(m_width), GLfloat(m_height), GLfloat(1.0f), GLfloat(1.0f)));
}

bool RenderTargetOpenGL::haveMips() const
{
	return false;
}

	}
}
