#pragma once

#include "Render/StructBuffer.h"
#include "Render/StructElement.h"

namespace traktor
{
	namespace render
	{

class ContextOpenGLES;

/*!
 * \ingroup OGL
 */
class StructBufferOpenGLES : public StructBuffer
{
	T_RTTI_CLASS;

public:
	StructBufferOpenGLES(ContextOpenGLES* context, const AlignedVector< StructElement >& structElements, uint32_t bufferSize);

	virtual ~StructBufferOpenGLES();

	virtual void destroy() override final;

	virtual void* lock() override final;

	virtual void unlock() override final;

	virtual const IBufferView* getBufferView() const override final;

private:
	Ref< ContextOpenGLES > m_context;
	GLuint m_buffer;
	uint8_t* m_lock;
};

	}
}