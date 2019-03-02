#pragma once

#include "Core/Misc/AutoPtr.h"
#include "Render/IndexBuffer.h"

namespace traktor
{
	namespace render
	{

class ContextOpenGLES2;
class StateCache;

/*!
 * \ingroup OGL
 */
class IndexBufferOpenGLES2 : public IndexBuffer
{
	T_RTTI_CLASS;

public:
	IndexBufferOpenGLES2(ContextOpenGLES2* context, IndexType indexType, uint32_t bufferSize, bool dynamic);

	virtual ~IndexBufferOpenGLES2();

	virtual void destroy() override final;

	virtual void* lock() override final;

	virtual void unlock() override final;

	void activate(StateCache* stateCache);

private:
	Ref< ContextOpenGLES2 > m_context;
	bool m_dynamic;
	GLuint m_name;
	AutoPtr< uint8_t, AllocFreeAlign > m_buffer;
};

	}
}

