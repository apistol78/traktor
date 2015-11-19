#include "Render/OpenGL/Std/ContextOpenGL.h"
#include "Render/OpenGL/Std/IndexBufferIBO.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

struct DeleteBufferCallback : public ContextOpenGL::IDeleteCallback
{
	GLuint m_bufferName;

	DeleteBufferCallback(GLuint bufferName)
	:	m_bufferName(bufferName)
	{
	}

	virtual void deleteResource()
	{
		T_OGL_SAFE(glDeleteBuffers(1, &m_bufferName));
		delete this;
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.IndexBufferIBO", IndexBufferIBO, IndexBufferOpenGL)

IndexBufferIBO::IndexBufferIBO(ContextOpenGL* resourceContext, IndexType indexType, uint32_t bufferSize, bool dynamic)
:	IndexBufferOpenGL(indexType, bufferSize)
,	m_resourceContext(resourceContext)
,	m_locked(false)
{
	T_OGL_SAFE(glGenBuffers(1, &m_name));
	T_OGL_SAFE(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_name));
	T_OGL_SAFE(glBufferData(GL_ELEMENT_ARRAY_BUFFER, bufferSize, 0, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW));
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
		if (m_resourceContext)
			m_resourceContext->deleteResource(new DeleteBufferCallback(m_name));
		m_name = 0;
	}
}

void* IndexBufferIBO::lock()
{
	T_ASSERT_M (!m_locked, L"Index buffer already locked");
	
	T_ANONYMOUS_VAR(ContextOpenGL::Scope)(m_resourceContext);
	T_OGL_SAFE(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_name));

	void* ptr = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
	if (ptr)
		m_locked = true;

	return ptr;
}

void IndexBufferIBO::unlock()
{
	T_ASSERT_M (m_locked, L"Index buffer not locked");

	T_ANONYMOUS_VAR(ContextOpenGL::Scope)(m_resourceContext);
	T_OGL_SAFE(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_name));
	T_OGL_SAFE(glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER));

	m_locked = false;
}

void IndexBufferIBO::bind()
{
	T_OGL_SAFE(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_name));
}

const GLvoid* IndexBufferIBO::getIndexData() const
{
	return 0;
}

	}
}
