#include "Render/OpenGL/ES/Platform.h"
#include "Render/OpenGL/ES/IndexBufferOpenGLES.h"
#include "Render/OpenGL/ES/StateCache.h"
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

struct DeleteBufferCallback : public ContextOpenGLES::IDeleteCallback
{
	GLuint m_bufferName;

	DeleteBufferCallback(GLuint bufferName)
	:	m_bufferName(bufferName)
	{
	}

	virtual ~DeleteBufferCallback()
	{
	}

	virtual void deleteResource()
	{
		T_OGL_SAFE(glDeleteBuffers(1, &m_bufferName));
		delete this;
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.IndexBufferOpenGLES", IndexBufferOpenGLES, IndexBuffer)

IndexBufferOpenGLES::IndexBufferOpenGLES(ContextOpenGLES* context, IndexType indexType, uint32_t bufferSize, bool dynamic)
:	IndexBuffer(indexType, bufferSize)
,	m_context(context)
,	m_dynamic(dynamic)
{
	T_OGL_SAFE(glGenBuffers(1, &m_name));
}

IndexBufferOpenGLES::~IndexBufferOpenGLES()
{
	destroy();
}

void IndexBufferOpenGLES::destroy()
{
	if (m_name)
	{
		if (m_context)
			m_context->deleteResource(new DeleteBufferCallback(m_name));
		m_name = 0;
	}
}

void* IndexBufferOpenGLES::lock()
{
	if (m_buffer.ptr())
		return 0;

	int32_t bufferSize = getBufferSize();
	m_buffer.reset((uint8_t*)Alloc::acquireAlign(bufferSize, 16, "IB"));

	return m_buffer.ptr();
}

void IndexBufferOpenGLES::unlock()
{
	if (!m_buffer.ptr())
		return;

	T_ANONYMOUS_VAR(ContextOpenGLES::Scope)(m_context);

	int32_t bufferSize = getBufferSize();
	T_OGL_SAFE(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_name));
	T_OGL_SAFE(glBufferData(GL_ELEMENT_ARRAY_BUFFER, bufferSize, m_buffer.ptr(), m_dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW));
	m_buffer.release();
}

void IndexBufferOpenGLES::activate(StateCache* stateCache)
{
	stateCache->setElementArrayBuffer(m_name);
}

	}
}
