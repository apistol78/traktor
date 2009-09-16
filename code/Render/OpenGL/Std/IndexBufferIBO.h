#ifndef traktor_render_IndexBufferIBO_H
#define traktor_render_IndexBufferIBO_H

#include "Core/Heap/Ref.h"
#include "Render/OpenGL/IndexBufferOpenGL.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_OPENGL_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class ContextOpenGL;

/*!
 * \ingroup OGL
 */
class T_DLLCLASS IndexBufferIBO : public IndexBufferOpenGL
{
	T_RTTI_CLASS(IndexBufferIBO)

public:
	IndexBufferIBO(ContextOpenGL* context, IndexType indexType, uint32_t bufferSize, bool dynamic);

	virtual ~IndexBufferIBO();

	virtual void destroy();

	virtual void* lock();
	
	virtual void unlock();

	virtual void bind();

	virtual const GLvoid* getIndexData() const;

private:
	Ref< ContextOpenGL > m_context;
	GLuint m_name;
	bool m_locked;
};
	
	}
}

#endif	// traktor_render_IndexBufferIBO_H
