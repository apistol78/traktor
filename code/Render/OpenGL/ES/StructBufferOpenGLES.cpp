#include <cstring>
#include "Core/Log/Log.h"
#include "Render/StructElement.h"
#include "Render/OpenGL/ES/Platform.h"
#include "Render/OpenGL/ES/StructBufferOpenGLES.h"

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
	GLuint m_buffer;

	DeleteBufferCallback(GLuint buffer)
	:	m_buffer(buffer)
	{
	}

	virtual void deleteResource()
	{
		T_OGL_SAFE(glDeleteBuffers(1, &m_buffer));
		delete this;
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.StructBufferOpenGLES", StructBufferOpenGLES, StructBuffer)

StructBufferOpenGLES::StructBufferOpenGLES(ContextOpenGLES* context, const AlignedVector< StructElement >& structElements, uint32_t bufferSize)
:	StructBuffer(bufferSize)
,	m_context(context)
,	m_buffer(0)
,	m_lock(nullptr)
{
	T_OGL_SAFE(glGenBuffers(1, &m_buffer));
#if !defined(__IOS__)
	T_OGL_SAFE(glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_buffer));
	T_OGL_SAFE(glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSize, nullptr, GL_DYNAMIC_COPY));
	T_OGL_SAFE(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
#endif
}

StructBufferOpenGLES::~StructBufferOpenGLES()
{
	destroy();
}

void StructBufferOpenGLES::destroy()
{
	if (m_buffer)
	{
		if (m_context)
			m_context->deleteResource(new DeleteBufferCallback(m_buffer));
		m_buffer = 0;
	}
}

void* StructBufferOpenGLES::lock()
{
	T_ASSERT_M(!m_lock, L"Struct buffer already locked");
	T_ANONYMOUS_VAR(ContextOpenGLES::Scope)(m_context);

#if !defined(__IOS__)
	T_OGL_SAFE(glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_buffer));
	m_lock = (uint8_t*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, getBufferSize(), GL_WRITE_ONLY);
#endif

	return m_lock;
}

void* StructBufferOpenGLES::lock(uint32_t structOffset, uint32_t structCount)
{
	return nullptr;
}

void StructBufferOpenGLES::unlock()
{
	T_ASSERT_M(m_lock, L"Struct buffer not locked");
	T_ANONYMOUS_VAR(ContextOpenGLES::Scope)(m_context);

#if !defined(__IOS__)
	T_OGL_SAFE(glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_buffer));
	T_OGL_SAFE(glUnmapBuffer(GL_SHADER_STORAGE_BUFFER));
#endif

	m_lock = nullptr;
}

	}
}