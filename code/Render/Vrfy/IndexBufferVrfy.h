#pragma once

#include "Render/IndexBuffer.h"

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup Vrfy
 */
class IndexBufferVrfy : public IndexBuffer
{
	T_RTTI_CLASS;

public:
	IndexBufferVrfy(IndexBuffer* indexBuffer, IndexType indexType, uint32_t bufferSize);

	virtual void destroy() override final;

	virtual void* lock() override final;

	virtual void unlock() override final;

	IndexBuffer* getIndexBuffer() const { return m_indexBuffer; }

private:
	Ref< IndexBuffer > m_indexBuffer;
	bool m_locked;
};

	}
}

