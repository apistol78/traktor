#ifndef traktor_render_VertexBufferDynamicOpenGLES2_H
#define traktor_render_VertexBufferDynamicOpenGLES2_H

#include "Core/Containers/AlignedVector.h"
#include "Core/Misc/AutoPtr.h"
#include "Render/OpenGL/ES2/VertexBufferOpenGLES2.h"

namespace traktor
{
	namespace render
	{

class IContext;
class StateCache;
class VertexElement;

/*!
 * \ingroup OGL
 */
class VertexBufferDynamicOpenGLES2 : public VertexBufferOpenGLES2
{
	T_RTTI_CLASS;

public:
	VertexBufferDynamicOpenGLES2(IContext* context, const std::vector< VertexElement >& vertexElements, uint32_t bufferSize);

	virtual ~VertexBufferDynamicOpenGLES2();

	virtual void destroy();

	virtual void* lock();

	virtual void* lock(uint32_t vertexOffset, uint32_t vertexCount);

	virtual void unlock();

	virtual void activate(StateCache* stateCache);

private:
	struct AttributeDesc
	{
		GLint location;
		GLint size;
		GLenum type;
		GLboolean normalized;
		GLuint offset;
	};

	Ref< IContext > m_context;
	AlignedVector< AttributeDesc > m_attributes;
	GLuint m_arrayObject;
	GLuint m_bufferObject;
	GLuint m_vertexStride;
	AutoPtr< uint8_t, AllocFreeAlign > m_buffer;
	GLintptr m_lockOffset;
	GLsizeiptr m_lockSize;
	bool m_dirty;
};

	}
}

#endif	// traktor_render_VertexBufferDynamicOpenGLES2_H
