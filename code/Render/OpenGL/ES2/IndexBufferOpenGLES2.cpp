#include "Render/OpenGL/Platform.h"
#include "Render/OpenGL/ES2/IndexBufferOpenGLES2.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.IndexBufferOpenGLES2", IndexBufferOpenGLES2, IndexBufferOpenGL)

IndexBufferOpenGLES2::IndexBufferOpenGLES2(IndexType indexType, uint32_t bufferSize, bool dynamic)
:	IndexBufferOpenGL(indexType, bufferSize)
{
	T_OGL_SAFE(glGenBuffers(1, &m_name));
	T_OGL_SAFE(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_name));
	T_OGL_SAFE(glBufferData(GL_ELEMENT_ARRAY_BUFFER, bufferSize, 0, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW));
}

IndexBufferOpenGLES2::~IndexBufferOpenGLES2()
{
	destroy();
}

void IndexBufferOpenGLES2::destroy()
{
	if (m_name)
	{
		T_OGL_SAFE(glDeleteBuffers(1, &m_name));
		m_name = 0;
	}
}

void* IndexBufferOpenGLES2::lock()
{
	return 0;
}

void IndexBufferOpenGLES2::unlock()
{
}

void IndexBufferOpenGLES2::bind()
{
	T_OGL_SAFE(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_name));
}

const GLvoid* IndexBufferOpenGLES2::getIndexData() const
{
	return 0;
}

	}
}
