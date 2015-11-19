#ifndef traktor_render_VertexBufferVAR_H
#define traktor_render_VertexBufferVAR_H

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
class VertexBufferVAR : public VertexBufferOpenGL
{
	T_RTTI_CLASS;

public:
	VertexBufferVAR(ContextOpenGL* resourceContext, const std::vector< VertexElement >& vertexElements, uint32_t bufferSize, bool dynamic);

	virtual ~VertexBufferVAR();

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
	bool m_dynamic;
	GLuint m_vertexStride;
	AttributeDesc m_attributeDesc[T_OGL_MAX_USAGE_INDEX];
	GLubyte* m_data;
};
	
	}
}

#endif	// traktor_render_VertexBufferVAR_H
