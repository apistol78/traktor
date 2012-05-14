#ifndef traktor_render_IndexBufferOpenGL_H
#define traktor_render_IndexBufferOpenGL_H

#include "Render/IndexBuffer.h"
#include "Render/OpenGL/Platform.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_OPENGL_EXPORT) || defined(T_RENDER_OPENGL_ES2_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup OGL
 */
class T_DLLCLASS IndexBufferOpenGL : public IndexBuffer
{
	T_RTTI_CLASS;

public:
	IndexBufferOpenGL(IndexType indexType, uint32_t bufferSize);

#if defined(T_OPENGL_STD)
	virtual void bind() = 0;

	virtual const GLvoid* getIndexData() const = 0;
#endif
};
	
	}
}

#endif	// traktor_render_IndexBufferOpenGL_H
