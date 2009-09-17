#ifndef traktor_render_IndexBufferOpenGLES2_H
#define traktor_render_IndexBufferOpenGLES2_H

#include "Core/Heap/Ref.h"
#include "Core/Misc/AutoPtr.h"
#include "Render/OpenGL/IndexBufferOpenGL.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_OPENGL_ES2_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class IContext;

/*!
 * \ingroup OGL
 */
class T_DLLCLASS IndexBufferOpenGLES2 : public IndexBufferOpenGL
{
	T_RTTI_CLASS(IndexBufferOpenGLES2)

public:
	IndexBufferOpenGLES2(IContext* context, IndexType indexType, uint32_t bufferSize, bool dynamic);

	virtual ~IndexBufferOpenGLES2();

	virtual void destroy();

	virtual void* lock();
	
	virtual void unlock();

	virtual void bind();

	virtual const GLvoid* getIndexData() const;

private:
	Ref< IContext > m_context;
	bool m_dynamic;
	GLuint m_name;
	AutoArrayPtr< uint8_t > m_buffer;
};
	
	}
}

#endif	// traktor_render_IndexBufferOpenGLES2_H
