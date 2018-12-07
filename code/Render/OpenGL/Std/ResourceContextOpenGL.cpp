#include <cstring>
#include "Core/Log/Log.h"
#include "Core/Misc/Adler32.h"
#include "Core/Misc/TString.h"
#include "Core/Thread/Acquire.h"
#include "Render/OpenGL/Std/ResourceContextOpenGL.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ResourceContextOpenGL", ResourceContextOpenGL, ContextOpenGL)

#if defined(_WIN32)
ResourceContextOpenGL::ResourceContextOpenGL(HWND hWnd, HDC hDC, HGLRC hRC)
:	ContextOpenGL(hWnd, hDC, hRC)
#elif defined(__APPLE__)
ResourceContextOpenGL::ResourceContextOpenGL(void* context)
:	ContextOpenGL(context)
#elif defined(__LINUX__)
ResourceContextOpenGL::ResourceContextOpenGL(::Display* display, ::Window window, GLXContext context)
:	ContextOpenGL(display, window, context)
#endif
,	m_maxAnisotropy(1.0f)
{
}

void ResourceContextOpenGL::setMaxAnisotropy(GLfloat maxAnisotropy)
{
	m_maxAnisotropy = maxAnisotropy;
}

GLuint ResourceContextOpenGL::createShaderObject(const char* shader, GLenum shaderType)
{
	char errorBuf[32000];
	GLsizei errorBufLen;
	GLint status;

	Adler32 adler;
	adler.begin();
	adler.feed(shader, std::strlen(shader));
	adler.end();

	uint32_t hash = adler.get();

	SmallMap< uint32_t, GLuint >::const_iterator i = m_shaderObjects.find(hash);
	if (i != m_shaderObjects.end())
		return i->second;

	GLuint shaderObject = glCreateShader(shaderType);
	T_OGL_SAFE(glShaderSource(shaderObject, 1, &shader, NULL));
	T_OGL_SAFE(glCompileShader(shaderObject));

	T_OGL_SAFE(glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &status));
	if (status != 1)
	{
		T_OGL_SAFE(glGetShaderInfoLog(shaderObject, sizeof(errorBuf), &errorBufLen, errorBuf));
		if (errorBufLen > 0)
		{
			log::error << L"GLSL shader compile failed :" << Endl;
			log::error << mbstows(errorBuf) << Endl;
			log::error << Endl;
			FormatMultipleLines(log::error, mbstows(shader));
			return 0;
		}
	}

	m_shaderObjects.insert(std::make_pair(hash, shaderObject));
	return shaderObject;
}

uint32_t ResourceContextOpenGL::createRenderStateObject(const RenderStateOpenGL& renderState)
{
	Adler32 adler;
	adler.feed(renderState.cullFaceEnable);
	adler.feed(renderState.cullFace);
	adler.feed(renderState.blendEnable);
	adler.feed(renderState.blendColorEquation);
	adler.feed(renderState.blendAlphaEquation);
	adler.feed(renderState.blendFuncColorSrc);
	adler.feed(renderState.blendFuncColorDest);
	adler.feed(renderState.blendFuncAlphaSrc);
	adler.feed(renderState.blendFuncAlphaDest);
	adler.feed(renderState.depthTestEnable);
	adler.feed(renderState.colorMask);
	adler.feed(renderState.depthMask);
	adler.feed(renderState.depthFunc);
	adler.feed(renderState.alphaTestEnable);
	adler.feed(renderState.alphaFunc);
	adler.feed(renderState.alphaRef);
	adler.feed(renderState.stencilTestEnable);
	adler.feed(renderState.stencilOpFail);
	adler.feed(renderState.stencilOpZFail);
	adler.feed(renderState.stencilOpZPass);

	SmallMap< uint32_t, uint32_t >::iterator i = m_renderStateListCache.find(adler.get());
	if (i != m_renderStateListCache.end())
		return i->second;

	uint32_t list = m_renderStateList.size() + 1;

	m_renderStateList.push_back(renderState);
	if (m_renderStateList.back().cullFace == GL_FRONT)
		m_renderStateList.back().cullFace = GL_BACK;
	else
		m_renderStateList.back().cullFace = GL_FRONT;

	m_renderStateList.push_back(renderState);
	if (m_renderStateList.back().cullFace == GL_FRONT)
		m_renderStateList.back().cullFace = GL_BACK;
	else
		m_renderStateList.back().cullFace = GL_FRONT;

	RenderStateOpenGL& rs = m_renderStateList.back();

	rs.depthTestEnable = GL_FALSE;
	rs.depthMask = GL_FALSE;
	rs.stencilTestEnable = GL_FALSE;

	m_renderStateListCache.insert(std::make_pair(adler.get(), list));
	return list;
}

uint32_t ResourceContextOpenGL::createSamplerStateObject(const SamplerStateOpenGL& samplerState)
{
	Adler32 adler;
	adler.feed(samplerState.minFilter);
	adler.feed(samplerState.magFilter);
	adler.feed(samplerState.wrapS);
	adler.feed(samplerState.wrapT);
	adler.feed(samplerState.wrapR);
	adler.feed(samplerState.compare);

	SmallMap< uint32_t, SamplerStateObject >::iterator i = m_samplerStateObjects.find(adler.get());
	if (i != m_samplerStateObjects.end())
		return i->first;

	SamplerStateObject& sso = m_samplerStateObjects[adler.get()];
	glGenSamplers(2, sso.samplers);

	for (uint32_t i = 0; i < sizeof_array(sso.samplers); ++i)
	{
		T_OGL_SAFE(glSamplerParameteri(sso.samplers[i], GL_TEXTURE_WRAP_S, samplerState.wrapS));
		T_OGL_SAFE(glSamplerParameteri(sso.samplers[i], GL_TEXTURE_WRAP_T, samplerState.wrapT));
		T_OGL_SAFE(glSamplerParameteri(sso.samplers[i], GL_TEXTURE_WRAP_R, samplerState.wrapR));

		T_OGL_SAFE(glSamplerParameteri(sso.samplers[i], GL_TEXTURE_MAG_FILTER, samplerState.magFilter));

		bool withMips = bool(i == 0);
		if (withMips)
		{
			T_OGL_SAFE(glSamplerParameteri(sso.samplers[i], GL_TEXTURE_MIN_FILTER, samplerState.minFilter));
			if (m_maxAnisotropy > 0.0f)
				T_OGL_SAFE(glSamplerParameteri(sso.samplers[i], GL_TEXTURE_MAX_ANISOTROPY_EXT, m_maxAnisotropy));
		}
		else
		{
			T_OGL_SAFE(glSamplerParameteri(sso.samplers[i], GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		}

		if (samplerState.compare != GL_INVALID_ENUM)
		{
			T_OGL_SAFE(glSamplerParameteri(sso.samplers[i], GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE));
			T_OGL_SAFE(glSamplerParameteri(sso.samplers[i], GL_TEXTURE_COMPARE_FUNC, samplerState.compare));
		}
		else
		{
			T_OGL_SAFE(glSamplerParameteri(sso.samplers[i], GL_TEXTURE_COMPARE_MODE, GL_NONE));
		}
	}

	return adler.get();
}

void ResourceContextOpenGL::deleteResource(IDeleteCallback* callback)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_deleteResources.push_back(callback);
}

void ResourceContextOpenGL::deleteResources()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	if (!m_deleteResources.empty())
	{
		for (AlignedVector< IDeleteCallback* >::iterator i = m_deleteResources.begin(); i != m_deleteResources.end(); ++i)
			(*i)->deleteResource();
		m_deleteResources.resize(0);
	}
}

	}
}
