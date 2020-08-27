#include "Render/OpenGL/ES/RenderTargetDepthOpenGLES.h"
#if defined(__ANDROID__)
#	include "Render/OpenGL/ES/Android/ContextOpenGLES.h"
#elif defined(__IOS__)
#	include "Render/OpenGL/ES/iOS/ContextOpenGLES.h"
#elif defined(__EMSCRIPTEN__)
#	include "Render/OpenGL/ES/Emscripten/ContextOpenGLES.h"
#elif defined(_WIN32)
#	include "Render/OpenGL/ES/Win32/ContextOpenGLES.h"
#elif defined(__LINUX__) || defined(__RPI__)
#	include "Render/OpenGL/ES/Linux/ContextOpenGLES.h"
#endif

namespace traktor
{
	namespace render
	{
		namespace
		{

struct DeleteTextureCallback : public ContextOpenGLES::IDeleteCallback
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

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderTargetDepthOpenGLES", RenderTargetDepthOpenGLES, ISimpleTexture)

RenderTargetDepthOpenGLES::RenderTargetDepthOpenGLES(ContextOpenGLES* resourceContext, GLuint depthTexture, int32_t width, int32_t height)
:	m_resourceContext(resourceContext)
,	m_depthTexture(depthTexture)
,	m_width(width)
,	m_height(height)
{
}

RenderTargetDepthOpenGLES::~RenderTargetDepthOpenGLES()
{
	destroy();
}

void RenderTargetDepthOpenGLES::destroy()
{
	if (m_depthTexture)
	{
		if (m_resourceContext)
			m_resourceContext->deleteResource(new DeleteTextureCallback(m_depthTexture));
		m_depthTexture = 0;
	}
}

ITexture* RenderTargetDepthOpenGLES::resolve()
{
	return this;
}

int32_t RenderTargetDepthOpenGLES::getMips() const
{
	return 1;
}

int32_t RenderTargetDepthOpenGLES::getWidth() const
{
	return m_width;
}

int32_t RenderTargetDepthOpenGLES::getHeight() const
{
	return m_height;
}

bool RenderTargetDepthOpenGLES::lock(int32_t level, Lock& lock)
{
	return false;
}

void RenderTargetDepthOpenGLES::unlock(int32_t level)
{
}

void* RenderTargetDepthOpenGLES::getInternalHandle()
{
	return (void*)m_depthTexture;
}

void RenderTargetDepthOpenGLES::bindSampler(GLuint unit, const SamplerStateOpenGL& samplerState, GLint locationTexture)
{
	T_OGL_SAFE(glActiveTexture(GL_TEXTURE0 + unit));
	T_OGL_SAFE(glBindTexture(GL_TEXTURE_2D, m_depthTexture));
	T_OGL_SAFE(glUniform1i(locationTexture, unit));

	//T_OGL_SAFE(glBindTexture(GL_TEXTURE_2D, m_depthTexture));
	//renderContext->bindSamplerStateObject(GL_TEXTURE_2D, samplerObject, stage, false);
}

void RenderTargetDepthOpenGLES::bindSize(GLint locationSize)
{
	T_OGL_SAFE(glUniform4f(locationSize, GLfloat(m_width), GLfloat(m_height), GLfloat(1.0f), GLfloat(1.0f)));
}

	}
}
