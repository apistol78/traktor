#include "Render/OpenGL/Std/IndexBufferIBO.h"
#include "Render/OpenGL/Std/ContextOpenGL.h"
#include "Render/OpenGL/Std/Extensions.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

struct DeleteBufferCallback : public ContextOpenGL::DeleteCallback
{
	GLuint m_bufferName;

	DeleteBufferCallback(GLuint bufferName)
	:	m_bufferName(bufferName)
	{
	}

	virtual void deleteResource()
	{
		T_OGL_SAFE(glDeleteBuffersARB(1, &m_bufferName));
		delete this;
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.IndexBufferIBO", IndexBufferIBO, IndexBufferOpenGL)

IndexBufferIBO::IndexBufferIBO(ContextOpenGL* context, IndexType indexType, uint32_t bufferSize, bool dynamic)
:	IndexBufferOpenGL(indexType, bufferSize)
,	m_context(context)
,	m_locked(false)
{
	T_OGL_SAFE(glGenBuffersARB(1, &m_name));
	T_OGL_SAFE(glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, m_name));
	T_OGL_SAFE(glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, bufferSize, 0, dynamic ? GL_DYNAMIC_DRAW_ARB : GL_STATIC_DRAW_ARB));
}

IndexBufferIBO::~IndexBufferIBO()
{
	destroy();
}

void IndexBufferIBO::destroy()
{
	T_ASSERT_M (!m_locked, L"Index buffer locked");
	if (m_name)
	{
		if (m_context)
			m_context->deleteResource(new DeleteBufferCallback(m_name));
		m_name = 0;
	}
}

void* IndexBufferIBO::lock()
{
	T_ASSERT_M (!m_locked, L"Index buffer already locked");
	
	T_CONTEXT_SCOPE(m_context);
	T_OGL_SAFE(glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, m_name));

	void* ptr = glMapBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, GL_WRITE_ONLY_ARB);
	if (ptr)
		m_locked = true;

	return ptr;
}

void IndexBufferIBO::unlock()
{
	T_ASSERT_M (m_locked, L"Index buffer not locked");

	T_CONTEXT_SCOPE(m_context);
	T_OGL_SAFE(glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, m_name));
	T_OGL_SAFE(glUnmapBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB));

	m_locked = false;
}

void IndexBufferIBO::bind()
{
	T_OGL_SAFE(glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, m_name));
}

const GLvoid* IndexBufferIBO::getIndexData() const
{
	return 0;
}

	}
}
