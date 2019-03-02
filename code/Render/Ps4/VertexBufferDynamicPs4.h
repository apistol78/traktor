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
class VertexBufferDynamicPs4 : public VertexBufferPs4
{
	T_RTTI_CLASS;

public:
	static Ref< VertexBufferDynamicPs4 > create(
		ContextPs4* context,
		uint32_t bufferSize,
		const AlignedVector< VertexElement >& vertexElements
	);

	virtual ~VertexBufferDynamicPs4();

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void* lock() T_OVERRIDE T_FINAL;

	virtual void* lock(uint32_t vertexOffset, uint32_t vertexCount) T_OVERRIDE T_FINAL;

	virtual void unlock() T_OVERRIDE T_FINAL;

	virtual void prepare() T_OVERRIDE T_FINAL;

private:
	Ref< ContextPs4 > m_context;

	VertexBufferDynamicPs4(uint32_t bufferSize);
};

	}
}

