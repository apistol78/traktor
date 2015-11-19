#ifndef traktor_render_IndexBufferOpenGLES2_H
#define traktor_render_IndexBufferOpenGLES2_H

#include "Core/Misc/AutoPtr.h"
#include "Render/OpenGL/IndexBufferOpenGL.h"

namespace traktor
{
	namespace render
	{

class ContextOpenGLES2;
class StateCache;

/*!
 * \ingroup OGL
 */
class IndexBufferOpenGLES2 : public IndexBufferOpenGL
{
	T_RTTI_CLASS;

public:
	IndexBufferOpenGLES2(ContextOpenGLES2* context, IndexType indexType, uint32_t bufferSize, bool dynamic);

	virtual ~IndexBufferOpenGLES2();

	virtual void destroy();

	virtual void* lock();
	
	virtual void unlock();

	void activate(StateCache* stateCache);

private:
	Ref< ContextOpenGLES2 > m_context;
	bool m_dynamic;
	GLuint m_name;
	AutoPtr< uint8_t, AllocFreeAlign > m_buffer;
};
	
	}
}

#endif	// traktor_render_IndexBufferOpenGLES2_H
