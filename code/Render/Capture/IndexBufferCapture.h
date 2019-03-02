#pragma once

#include "Render/IndexBuffer.h"

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup Capture
 */
class IndexBufferCapture : public IndexBuffer
{
	T_RTTI_CLASS;

public:
	IndexBufferCapture(IndexBuffer* indexBuffer, IndexType indexType, uint32_t bufferSize);

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

