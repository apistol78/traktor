#ifndef traktor_render_VertexBufferStaticVBO_H
#define traktor_render_VertexBufferStaticVBO_H

#include "Render/OpenGL/VertexBufferOpenGL.h"

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
class VertexElement;

/*!
 * \ingroup OGL
 */
class T_DLLCLASS VertexBufferStaticVBO : public VertexBufferOpenGL
{
	T_RTTI_CLASS;

public:
	VertexBufferStaticVBO(IContext* resourceContext, const std::vector< VertexElement >& vertexElements, uint32_t bufferSize);

	virtual ~VertexBufferStaticVBO();

	virtual void destroy();

	virtual void* lock();

	virtual void* lock(uint32_t vertexOffset, uint32_t vertexCount);

	virtual void unlock();

	virtual void activate(const GLint* attributeLocs);

private:
	struct AttributeDesc
	{
		GLint size;
		GLenum type;
		GLboolean normalized;
		GLuint offset;
	};

	Ref< IContext > m_resourceContext;
	GLuint m_name;
	GLuint m_vertexStride;
	AttributeDesc m_attributeDesc[T_OGL_MAX_USAGE_INDEX];
	uint8_t* m_lock;
};

	}
}

#endif	// traktor_render_VertexBufferStaticVBO_H
