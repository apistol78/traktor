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

void RenderTargetOpenGL::bindSampler(ContextOpenGL* renderContext, GLuint unit, const SamplerState& samplerState, GLint locationTexture)
{
	T_OGL_SAFE(glActiveTexture(GL_TEXTURE0 + unit));
	T_OGL_SAFE(glBindTexture(GL_TEXTURE_2D, m_colorTexture));

	if (!opengl_have_extension(E_T_rendertarget_nearest_filter_only))
		renderContext->setSamplerState(unit, samplerState, 0);

	T_OGL_SAFE(glUniform1iARB(locationTexture, unit));
}

void RenderTargetOpenGL::bindSize(GLint locationSize)
{
	T_OGL_SAFE(glUniform4fARB(locationSize, GLfloat(m_width), GLfloat(m_height), GLfloat(1.0f), GLfloat(1.0f)));
}

	}
}
