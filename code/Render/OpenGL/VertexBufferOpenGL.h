#ifndef traktor_render_VertexBufferOpenGL_H
#define traktor_render_VertexBufferOpenGL_H

#include "Render/VertexBuffer.h"

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

class VertexElement;

/*!
 * \ingroup OGL
 */
class T_DLLCLASS VertexBufferOpenGL : public VertexBuffer
{
	T_RTTI_CLASS(VertexBufferOpenGL)

public:
	VertexBufferOpenGL(uint32_t bufferSize);

	virtual void activate(const GLint* attributeLocs) = 0;
};
	
	}
}

#endif	// traktor_render_VertexBufferOpenGL_H
