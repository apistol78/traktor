#pragma once

#include "Render/VertexElement.h"
#include "Render/Ps4/VertexBufferPs4.h"

namespace traktor
{
	namespace render
	{

class ContextPs4;

/*!
 * \ingroup GNM
 */
class VertexBufferStaticPs4 : public VertexBufferPs4
{
	T_RTTI_CLASS;

public:
	static Ref< VertexBufferStaticPs4 > create(
		ContextPs4* context,
		uint32_t bufferSize,
		const AlignedVector< VertexElement >& vertexElements
	);

	virtual ~VertexBufferStaticPs4();

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void* lock() T_OVERRIDE T_FINAL;

	virtual void unlock() T_OVERRIDE T_FINAL;

	virtual void prepare() T_OVERRIDE T_FINAL;

private:
	Ref< ContextPs4 > m_context;

	VertexBufferStaticPs4(uint32_t bufferSize);
};

	}
}

