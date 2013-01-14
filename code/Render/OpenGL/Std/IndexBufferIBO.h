#ifndef traktor_render_IndexBufferIBO_H
#define traktor_render_IndexBufferIBO_H

#include "Render/OpenGL/IndexBufferOpenGL.h"

namespace traktor
{
	namespace render
	{

class IContext;

/*!
 * \ingroup OGL
 */
class IndexBufferIBO : public IndexBufferOpenGL
{
	T_RTTI_CLASS;

public:
	IndexBufferIBO(IContext* resourceContext, IndexType indexType, uint32_t bufferSize, bool dynamic);

	virtual ~IndexBufferIBO();

	virtual void destroy();

	virtual void* lock();
	
	virtual void unlock();

	virtual void bind();

	virtual const GLvoid* getIndexData() const;

private:
	Ref< IContext > m_resourceContext;
	GLuint m_name;
	bool m_locked;
};
	
	}
}

#endif	// traktor_render_IndexBufferIBO_H
