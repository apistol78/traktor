#include <cstring>
#include "Core/Platform.h"
#include "Core/RefArray.h"
#include "Core/Log/Log.h"
#include "Core/Misc/Adler32.h"
#include "Core/Misc/TString.h"
#include "Core/Thread/Acquire.h"
#include "Render/OpenGL/ES/ExtensionsGLES.h"
#include "Render/OpenGL/ES/iOS/ContextOpenGLES.h"
#include "Render/OpenGL/ES/iOS/EAGLContextWrapper.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

const uint32_t c_maxMatchConfigs = 64;

typedef RefArray< ContextOpenGLES > context_stack_t;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ContextOpenGLES", ContextOpenGLES, Object)

ThreadLocal ContextOpenGLES::ms_contextStack;

Ref< ContextOpenGLES > ContextOpenGLES::createContext(const SystemApplication& sysapp, const RenderViewDefaultDesc& desc)
{
	return nullptr;
}

Ref< ContextOpenGLES > ContextOpenGLES::createContext(const SystemApplication& sysapp, const RenderViewEmbeddedDesc& desc)
{
	Ref< ContextOpenGLES > context = new ContextOpenGLES();

	context->m_context = new EAGLContextWrapper();
	if (!context->m_context->create(desc.syswin.view))
		return nullptr;

	if (!context->enter())
		return nullptr;
	initializeExtensions();
	context->leave();

	log::info << L"OpenGL ES render context created successfully (embedded)" << Endl;
	return context;
}

bool ContextOpenGLES::reset(int32_t width, int32_t height)
{
	return true;
}

bool ContextOpenGLES::enter()
{
	if (!m_lock.wait())
		return false;

	context_stack_t* stack = static_cast< context_stack_t* >(ms_contextStack.get());
	if (!stack)
	{
		stack = new context_stack_t();
		ms_contextStack.set(stack);
	}

	if (!EAGLContextWrapper::setCurrent(m_context))
	{
		m_lock.release();
		return false;
	}

	stack->push_back(this);
	return true;
}

void ContextOpenGLES::leave()
{
	context_stack_t* stack = static_cast< context_stack_t* >(ms_contextStack.get());

	T_ASSERT (stack);
	T_ASSERT (!stack->empty());
	T_ASSERT (stack->back() == this);

	stack->pop_back();

	if (!stack->empty())
		EAGLContextWrapper::setCurrent(stack->back()->m_context);
	else
		EAGLContextWrapper::setCurrent(0);

	m_lock.release();
}

void ContextOpenGLES::deleteResource(IDeleteCallback* callback)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_deleteResources.push_back(callback);
}

void ContextOpenGLES::deleteResources()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	if (!m_deleteResources.empty())
	{
		enter();
		for (std::vector< IDeleteCallback* >::iterator i = m_deleteResources.begin(); i != m_deleteResources.end(); ++i)
			(*i)->deleteResource();
		m_deleteResources.resize(0);
		leave();
	}
}

GLuint ContextOpenGLES::createShaderObject(const char* shader, GLenum shaderType)
{
	Adler32 adler;
	adler.begin();
	adler.feed(shader, strlen(shader));
	adler.end();

	uint32_t hash = adler.get();

	std::map< uint32_t, GLuint >::const_iterator i = m_shaderObjects.find(hash);
	if (i != m_shaderObjects.end())
		return i->second;

	GLuint shaderObject = glCreateShader(shaderType);
	if (shaderObject == 0)
	{
		log::error << L"Failed to compile GLSL shader; glCreateShader returned 0" << Endl;
		return 0;
	}

	T_OGL_SAFE(glShaderSource(shaderObject, 1, &shader, NULL));
	T_OGL_SAFE(glCompileShader(shaderObject));

#if defined(_DEBUG)
	char errorBuf[32000];
	GLint status;

	T_OGL_SAFE(glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &status));
	if (status == 0)
	{
		T_OGL_SAFE(glGetShaderInfoLog(shaderObject, sizeof(errorBuf), 0, errorBuf));
		log::error << L"Failed to compile GLSL shader:" << Endl;
		log::error << mbstows(errorBuf) << Endl;
		log::error << Endl;
		FormatMultipleLines(log::error, mbstows(shader));
		return 0;
	}
#endif

	m_shaderObjects.insert(std::make_pair(hash, shaderObject));
	return shaderObject;
}

int32_t ContextOpenGLES::getWidth() const
{
	return m_context->getWidth();
}

int32_t ContextOpenGLES::getHeight() const
{
	return m_context->getHeight();
}

void ContextOpenGLES::swapBuffers()
{
	m_context->swapBuffers();
}

Semaphore& ContextOpenGLES::lock()
{
	return m_lock;
}

void ContextOpenGLES::bindPrimary()
{
	T_OGL_SAFE(glBindFramebuffer(GL_FRAMEBUFFER, m_context->getFrameBuffer()));
	T_OGL_SAFE(glViewport(
		0,
		0,
		getWidth(),
		getHeight()
	));
}

GLuint ContextOpenGLES::getPrimaryDepth() const
{
	return m_context->getDepthBuffer();
}

ContextOpenGLES::ContextOpenGLES()
:	m_context(nullptr)
{
}

	}
}
