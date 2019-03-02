#pragma once

#include "Render/VertexBuffer.h"

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup GNM
 */
class VertexBufferPs4 : public VertexBuffer
{
	T_RTTI_CLASS;

public:
	virtual void prepare();

protected:
	VertexBufferPs4(uint32_t bufferSize);
};

	}
}

