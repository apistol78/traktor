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
		namespace
		{
		
const GLenum c_glFilter[] =
{
	GL_NEAREST,
	GL_LINEAR
};

const GLenum c_glWrap[] =
{
	GL_REPEAT,
	GL_REPEAT,
	GL_CLAMP_TO_EDGE,
	GL_CLAMP_TO_EDGE
};

const GLenum c_glCompare[] =
{
	GL_ALWAYS,
	GL_NEVER,
	GL_LESS,
	GL_LEQUAL,
	GL_GREATER,
	GL_GEQUAL,
	GL_EQUAL,
	GL_NOTEQUAL,
	GL_INVALID_ENUM
};

		}

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

uint32_t ResourceContextOpenGL::createRenderStateObject(const RenderState& renderState)
{
	Adler32 adler;
	adler.feed(renderState);

	auto it = m_renderStateListCache.find(adler.get());
	if (it != m_renderStateListCache.end())
		return it->second;

	uint32_t list = m_renderStateList.size() + 1;

	m_renderStateList.push_back(renderState);
	m_renderStateListCache.insert(std::make_pair(adler.get(), list));
	return list;
}

uint32_t ResourceContextOpenGL::createSamplerStateObject(const SamplerState& samplerState)
{
	Adler32 adler;
	adler.feed(samplerState);

	auto it = m_samplerStateObjects.find(adler.get());
	if (it != m_samplerStateObjects.end())
		return it->first;

	auto& sso = m_samplerStateObjects[adler.get()];
	glGenSamplers(2, sso.samplers);

	for (uint32_t i = 0; i < sizeof_array(sso.samplers); ++i)
	{
		// Texture filtering.
		if (i == 0)
		{
			bool minLinear = samplerState.minFilter != FtPoint;
			bool mipLinear = samplerState.mipFilter != FtPoint;

			if (!minLinear && !mipLinear)
				{ T_OGL_SAFE(glSamplerParameteri(sso.samplers[i], GL_TEXTURE_MIN_FILTER, GL_NEAREST)); }
			else if (!minLinear && mipLinear)
				{ T_OGL_SAFE(glSamplerParameteri(sso.samplers[i], GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR)); }
			else if (minLinear && !mipLinear)
				{ T_OGL_SAFE(glSamplerParameteri(sso.samplers[i], GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST)); }
			else
				{ T_OGL_SAFE(glSamplerParameteri(sso.samplers[i], GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR)); }

	 		if (m_maxAnisotropy > 0.0f)
	 			{ T_OGL_SAFE(glSamplerParameteri(sso.samplers[i], GL_TEXTURE_MAX_ANISOTROPY_EXT, m_maxAnisotropy)); }
		}
		else
		{
			bool minLinear = samplerState.minFilter != FtPoint;
			if (!minLinear)
				{ T_OGL_SAFE(glSamplerParameteri(sso.samplers[i], GL_TEXTURE_MIN_FILTER, GL_NEAREST)); }
			else
				{ T_OGL_SAFE(glSamplerParameteri(sso.samplers[i], GL_TEXTURE_MIN_FILTER, GL_LINEAR)); }
		}

	 	T_OGL_SAFE(glSamplerParameteri(sso.samplers[i], GL_TEXTURE_MAG_FILTER, c_glFilter[samplerState.magFilter]));

		// Texture address mode.
	 	T_OGL_SAFE(glSamplerParameteri(sso.samplers[i], GL_TEXTURE_WRAP_S, c_glWrap[samplerState.addressU]));
	 	T_OGL_SAFE(glSamplerParameteri(sso.samplers[i], GL_TEXTURE_WRAP_T, c_glWrap[samplerState.addressV]));
	 	T_OGL_SAFE(glSamplerParameteri(sso.samplers[i], GL_TEXTURE_WRAP_R, c_glWrap[samplerState.addressW]));

		// Shadow compare operation.
		GLenum compare = c_glCompare[samplerState.compare];
	 	if (compare != GL_INVALID_ENUM)
	 	{
	 		T_OGL_SAFE(glSamplerParameteri(sso.samplers[i], GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE));
	 		T_OGL_SAFE(glSamplerParameteri(sso.samplers[i], GL_TEXTURE_COMPARE_FUNC, compare));
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
		for (auto deleteResource : m_deleteResources)
			deleteResource->deleteResource();
		m_deleteResources.resize(0);
	}
}

	}
}
