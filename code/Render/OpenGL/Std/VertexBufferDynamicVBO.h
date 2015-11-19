#ifndef traktor_render_VertexBufferDynamicVBO_H
#define traktor_render_VertexBufferDynamicVBO_H

#include "Core/Containers/AlignedVector.h"
#include "Render/OpenGL/VertexBufferOpenGL.h"

namespace traktor
{
	namespace render
	{

class ContextOpenGL;
class VertexElement;

/*!
 * \ingroup OGL
 */
class VertexBufferDynamicVBO : public VertexBufferOpenGL
{
	T_RTTI_CLASS;

public:
	VertexBufferDynamicVBO(ContextOpenGL* resourceContext, const std::vector< VertexElement >& vertexElements, uint32_t bufferSize);

	virtual ~VertexBufferDynamicVBO();

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void* lock() T_OVERRIDE T_FINAL;

	virtual void* lock(uint32_t vertexOffset, uint32_t vertexCount) T_OVERRIDE T_FINAL;

	virtual void unlock() T_OVERRIDE T_FINAL;

	virtual void activate(const GLint* attributeLocs) T_OVERRIDE T_FINAL;

private:
	struct AttributeDesc
	{
		GLint size;
		GLenum type;
		GLboolean normalized;
		GLuint offset;
	};

	Ref< ContextOpenGL > m_resourceContext;
	GLuint m_array;
	GLuint m_buffer;
	GLuint m_vertexStride;
	AttributeDesc m_attributeDesc[T_OGL_MAX_USAGE_INDEX];
	const GLint* m_attributeLocs;
	uint8_t* m_lock;
	GLvoid* m_mapped;
	AlignedVector< uint8_t > m_data;
	bool m_dirty;
};

	}
}

#endif	// traktor_render_VertexBufferDynamicVBO_H
