#include <cstring>
#include "Core/Log/Log.h"
#include "Render/StructElement.h"
#include "Render/OpenGL/Std/ResourceContextOpenGL.h"
#include "Render/OpenGL/Std/StructBufferOpenGL.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

struct DeleteBufferCallback : public ResourceContextOpenGL::IDeleteCallback
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

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.StructBufferOpenGL", StructBufferOpenGL, StructBuffer)

StructBufferOpenGL::StructBufferOpenGL(ResourceContextOpenGL* resourceContext, const AlignedVector< StructElement >& structElements, uint32_t bufferSize)
:	StructBuffer(bufferSize)
,	m_resourceContext(resourceContext)
,	m_buffer(0)
,	m_lock(nullptr)
{
	T_OGL_SAFE(glGenBuffers(1, &m_buffer));
	T_OGL_SAFE(glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_buffer));
	T_OGL_SAFE(glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSize, nullptr, GL_DYNAMIC_COPY));
	T_OGL_SAFE(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));
}

StructBufferOpenGL::~StructBufferOpenGL()
{
	destroy();
}

void StructBufferOpenGL::destroy()
{
	if (m_buffer)
	{
		if (m_resourceContext)
			m_resourceContext->deleteResource(new DeleteBufferCallback(m_buffer));
		m_buffer = 0;
	}
}

void* StructBufferOpenGL::lock()
{
	T_ASSERT_M(!m_lock, L"Vertex buffer already locked");
	T_ANONYMOUS_VAR(ContextOpenGL::Scope)(m_resourceContext);

	T_OGL_SAFE(glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_buffer));
	m_lock = (uint8_t*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
	
	return m_lock;
}

void* StructBufferOpenGL::lock(uint32_t structOffset, uint32_t structCount)
{
	return nullptr;
}

void StructBufferOpenGL::unlock()
{
	T_ASSERT_M(m_lock, L"Vertex buffer not locked");
	T_ANONYMOUS_VAR(ContextOpenGL::Scope)(m_resourceContext);

	T_OGL_SAFE(glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_buffer));
	T_OGL_SAFE(glUnmapBuffer(GL_SHADER_STORAGE_BUFFER));
	T_OGL_SAFE(glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0));	
}

	}
}