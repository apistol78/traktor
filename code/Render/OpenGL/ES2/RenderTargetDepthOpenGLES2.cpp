#include "Render/OpenGL/ES2/RenderTargetDepthOpenGLES2.h"
#if defined(__ANDROID__)
#	include "Render/OpenGL/ES2/Android/ContextOpenGLES2.h"
#elif defined(__IOS__)
#	include "Render/OpenGL/ES2/iOS/EAGLContextWrapper.h"
#elif defined(__EMSCRIPTEN__)
#	include "Render/OpenGL/ES2/Emscripten/ContextOpenGLES2.h"
#elif defined(__PNACL__)
#	include "Render/OpenGL/ES2/PNaCl/ContextOpenGLES2.h"
#elif defined(_WIN32)
#	include "Render/OpenGL/ES2/Win32/ContextOpenGLES2.h"
#endif

namespace traktor
{
	namespace render
	{
		namespace
		{

struct DeleteTextureCallback : public ContextOpenGLES2::IDeleteCallback
{
	GLuint m_textureName;

	DeleteTextureCallback(GLuint textureName)
	:	m_textureName(textureName)
	{
	}

	virtual ~DeleteTextureCallback()
	{
	}

	virtual void deleteResource()
	{
		T_OGL_SAFE(glDeleteTextures(1, &m_textureName));
		delete this;
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderTargetDepthOpenGLES2", RenderTargetDepthOpenGLES2, ISimpleTexture)

RenderTargetDepthOpenGLES2::RenderTargetDepthOpenGLES2(ContextOpenGLES2* resourceContext, GLuint depthTexture, int32_t width, int32_t height)
:	m_resourceContext(resourceContext)
,	m_depthTexture(depthTexture)
,	m_width(width)
,	m_height(height)
{
}

RenderTargetDepthOpenGLES2::~RenderTargetDepthOpenGLES2()
{
	destroy();
}

void RenderTargetDepthOpenGLES2::destroy()
{
	if (m_depthTexture)
	{
		if (m_resourceContext)
			m_resourceContext->deleteResource(new DeleteTextureCallback(m_depthTexture));
		m_depthTexture = 0;
	}
}

ITexture* RenderTargetDepthOpenGLES2::resolve()
{
	return this;
}

int RenderTargetDepthOpenGLES2::getWidth() const
{
	return m_width;
}

int RenderTargetDepthOpenGLES2::getHeight() const
{
	return m_height;
}

bool RenderTargetDepthOpenGLES2::lock(int level, Lock& lock)
{
	return false;
}

void RenderTargetDepthOpenGLES2::unlock(int level)
{
}

void* RenderTargetDepthOpenGLES2::getInternalHandle()
{
	return (void*)m_depthTexture;
}

void RenderTargetDepthOpenGLES2::bindSampler(GLuint unit, const SamplerStateOpenGL& samplerState, GLint locationTexture)
{
	T_OGL_SAFE(glActiveTexture(GL_TEXTURE0 + unit));
	T_OGL_SAFE(glBindTexture(GL_TEXTURE_2D, m_depthTexture));
	T_OGL_SAFE(glUniform1i(locationTexture, unit));

	//T_OGL_SAFE(glBindTexture(GL_TEXTURE_2D, m_depthTexture));
	//renderContext->bindSamplerStateObject(GL_TEXTURE_2D, samplerObject, stage, false);
}

void RenderTargetDepthOpenGLES2::bindSize(GLint locationSize)
{
	T_OGL_SAFE(glUniform4f(locationSize, GLfloat(m_width), GLfloat(m_height), GLfloat(1.0f), GLfloat(1.0f)));
}

	}
}
