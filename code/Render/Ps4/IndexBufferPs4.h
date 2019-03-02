#pragma once

#include "Render/IndexBuffer.h"

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup GNM
 */
class IndexBufferPs4 : public IndexBuffer
{
	T_RTTI_CLASS;

public:
	IndexBufferPs4(IndexType indexType, uint32_t bufferSize);

	virtual void prepare();
};

	}
}

