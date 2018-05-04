/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <cstring>
#include "Render/OpenGL/ES2/RenderTargetOpenGLES2.h"
#if defined(__ANDROID__)
#	include "Render/OpenGL/ES2/Android/ContextOpenGLES2.h"
#elif defined(__IOS__)
#	include "Render/OpenGL/ES2/iOS/ContextOpenGLES2.h"
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

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderTargetOpenGLES2", RenderTargetOpenGLES2, ISimpleTexture)

RenderTargetOpenGLES2::RenderTargetOpenGLES2(ContextOpenGLES2* context, GLuint colorTexture, int32_t width, int32_t height)
:	m_context(context)
,	m_colorTexture(colorTexture)
,	m_width(width)
,	m_height(height)
{
	std::memset(&m_shadowState, 0, sizeof(m_shadowState));
}

RenderTargetOpenGLES2::~RenderTargetOpenGLES2()
{
	destroy();
}

void RenderTargetOpenGLES2::destroy()
{
	if (m_colorTexture)
	{
		if (m_context)
			m_context->deleteResource(new DeleteTextureCallback(m_colorTexture));
		m_colorTexture = 0;
	}
}

ITexture* RenderTargetOpenGLES2::resolve()
{
	return this;
}

int RenderTargetOpenGLES2::getWidth() const
{
	return m_width;
}

int RenderTargetOpenGLES2::getHeight() const
{
	return m_height;
}

bool RenderTargetOpenGLES2::lock(int level, Lock& lock)
{
	return false;
}

void RenderTargetOpenGLES2::unlock(int level)
{
}

void* RenderTargetOpenGLES2::getInternalHandle()
{
	return (void*)m_colorTexture;
}

void RenderTargetOpenGLES2::bindSampler(GLuint unit, const SamplerStateOpenGL& samplerState, GLint locationTexture)
{
	T_OGL_SAFE(glActiveTexture(GL_TEXTURE0 + unit));
	T_OGL_SAFE(glBindTexture(GL_TEXTURE_2D, m_colorTexture));

	GLenum minFilter = GL_NEAREST;
	if (samplerState.minFilter != GL_NEAREST)
		minFilter = GL_LINEAR;
	else
		minFilter = GL_NEAREST;

	if (m_shadowState.minFilter != minFilter)
	{
		T_OGL_SAFE(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter));
		m_shadowState.minFilter = minFilter;
	}

	if (m_shadowState.magFilter != samplerState.magFilter)
	{
		T_OGL_SAFE(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, samplerState.magFilter));
		m_shadowState.magFilter = samplerState.magFilter;
	}

	if (m_shadowState.wrapS != samplerState.wrapS)
	{
		T_OGL_SAFE(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, samplerState.wrapS));
		m_shadowState.wrapS = samplerState.wrapS;
	}

	if (m_shadowState.wrapT != samplerState.wrapT)
	{
		T_OGL_SAFE(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, samplerState.wrapT));
		m_shadowState.wrapT = samplerState.wrapT;
	}

	T_OGL_SAFE(glUniform1i(locationTexture, unit));
}

void RenderTargetOpenGLES2::bindSize(GLint locationSize)
{
	T_OGL_SAFE(glUniform4f(locationSize, GLfloat(m_width), GLfloat(m_height), GLfloat(1.0f), GLfloat(1.0f)));
}

	}
}
