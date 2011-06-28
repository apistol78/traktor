#ifndef traktor_render_VertexBufferOpenGLES2_H
#define traktor_render_VertexBufferOpenGLES2_H

#include "Core/Containers/AlignedVector.h"
#include "Core/Misc/AutoPtr.h"
#include "Render/OpenGL/VertexBufferOpenGL.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_OPENGL_ES2_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

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
class T_DLLCLASS VertexBufferOpenGLES2 : public VertexBufferOpenGL
{
	T_RTTI_CLASS;

public:
	VertexBufferOpenGLES2(IContext* context, const std::vector< VertexElement >& vertexElements, uint32_t bufferSize, bool dynamic);

	virtual ~VertexBufferOpenGLES2();

	virtual void destroy();

	virtual void* lock();

	virtual void* lock(uint32_t vertexOffset, uint32_t vertexCount);

	virtual void unlock();

	void activate(StateCache* stateCache);

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
	bool m_dynamic;
	GLuint m_arrayObject;
	GLuint m_bufferObject;
	GLuint m_vertexStride;
	AutoPtr< uint8_t, AllocFreeAlign > m_buffer;
	GLintptr m_lockOffset;
	GLsizeiptr m_lockSize;
};

	}
}

#endif	// traktor_render_VertexBufferOpenGLES2_H
