#pragma once

#include "Render/StructBuffer.h"
#include "Render/StructElement.h"
#include "Render/OpenGL/Std/BufferViewOpenGL.h"

namespace traktor
{
	namespace render
	{

class ResourceContextOpenGL;

/*!
 * \ingroup OGL
 */
class StructBufferOpenGL : public StructBuffer
{
	T_RTTI_CLASS;

public:
	StructBufferOpenGL(ResourceContextOpenGL* resourceContext, const AlignedVector< StructElement >& structElements, uint32_t bufferSize);

	virtual ~StructBufferOpenGL();

	virtual void destroy() override final;

	virtual void* lock() override final;

	virtual void unlock() override final;

	virtual const IBufferView* getBufferView() const override final;

private:
	Ref< ResourceContextOpenGL > m_resourceContext;
	GLuint m_buffer;
	BufferViewOpenGL m_bufferView;
	uint8_t* m_lock;
};

	}
}