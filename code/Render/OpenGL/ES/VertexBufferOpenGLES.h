#pragma once

#include "Render/VertexBuffer.h"

namespace traktor
{
	namespace render
	{

class StateCache;

/*!
 * \ingroup OGL
 */
class VertexBufferOpenGLES : public VertexBuffer
{
	T_RTTI_CLASS;

public:
	VertexBufferOpenGLES(uint32_t bufferSize);

	virtual void activate(StateCache* stateCache) = 0;
};

	}
}

