#ifndef traktor_render_IndexBufferIAR_H
#define traktor_render_IndexBufferIAR_H

#include "Render/OpenGL/IndexBufferOpenGL.h"

namespace traktor
{
	namespace render
	{

class IContext;

/*!
 * \ingroup OGL
 */
class IndexBufferIAR : public IndexBufferOpenGL
{
	T_RTTI_CLASS;

public:
	IndexBufferIAR(IContext* resourceContext, IndexType indexType, uint32_t bufferSize);

	virtual ~IndexBufferIAR();

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void* lock() T_OVERRIDE T_FINAL;
	
	virtual void unlock() T_OVERRIDE T_FINAL;

	virtual void bind() T_OVERRIDE T_FINAL;

	virtual const GLvoid* getIndexData() const T_OVERRIDE T_FINAL;
	
private:
	Ref< IContext > m_resourceContext;
	GLubyte* m_data;
};
	
	}
}

#endif	// traktor_render_IndexBufferIAR_H
