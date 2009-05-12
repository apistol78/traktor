#ifndef traktor_render_VertexBufferVBO_H
#define traktor_render_VertexBufferVBO_H

#include "Core/Heap/Ref.h"
#include "Render/OpenGL/VertexBufferOpenGL.h"

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
class VertexElement;

/*!
 * \ingroup OGL
 */
class T_DLLCLASS VertexBufferVBO : public VertexBufferOpenGL
{
	T_RTTI_CLASS(VertexBufferVBO)

public:
	VertexBufferVBO(ContextOpenGL* context, const std::vector< VertexElement >& vertexElements, uint32_t bufferSize, bool dynamic);

	virtual ~VertexBufferVBO();

	virtual void destroy();

	virtual void* lock();

	virtual void* lock(uint32_t vertexOffset, uint32_t vertexCount);

	virtual void unlock();

	void activate(const GLint* attributeLocs);

private:
	struct AttributeDesc
	{
		GLint size;
		GLenum type;
		GLboolean normalized;
		GLuint offset;
	};

	Ref< ContextOpenGL > m_context;
	GLuint m_name;
	GLuint m_vertexStride;
	AttributeDesc m_attributeDesc[T_OGL_MAX_USAGE_INDEX];
	bool m_locked;
};

	}
}

#endif	// traktor_render_VertexBufferVBO_H
