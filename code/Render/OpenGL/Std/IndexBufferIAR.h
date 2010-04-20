#ifndef traktor_render_IndexBufferIAR_H
#define traktor_render_IndexBufferIAR_H

#include "Render/OpenGL/IndexBufferOpenGL.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_OPENGL_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class IContext;

/*!
 * \ingroup OGL
 */
class T_DLLCLASS IndexBufferIAR : public IndexBufferOpenGL
{
	T_RTTI_CLASS;

public:
	IndexBufferIAR(IContext* resourceContext, IndexType indexType, uint32_t bufferSize);

	virtual ~IndexBufferIAR();

	virtual void destroy();

	virtual void* lock();
	
	virtual void unlock();

	virtual void bind();

	virtual const GLvoid* getIndexData() const;
	
private:
	Ref< IContext > m_resourceContext;
	GLubyte* m_data;
};
	
	}
}

#endif	// traktor_render_IndexBufferIAR_H
