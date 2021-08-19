#pragma once

#include "Render/IndexBuffer.h"

namespace traktor
{
	namespace render
	{

class ResourceTracker;

/*!
 * \ingroup Vrfy
 */
class IndexBufferVrfy : public IndexBuffer
{
	T_RTTI_CLASS;

public:
	IndexBufferVrfy(ResourceTracker* resourceTracker, IndexBuffer* indexBuffer, IndexType indexType, uint32_t bufferSize);

	virtual ~IndexBufferVrfy();

	virtual void destroy() override final;

	virtual void* lock() override final;

	virtual void unlock() override final;

	virtual const IBufferView* getBufferView() const override final;

	IndexBuffer* getIndexBuffer() const { return m_indexBuffer; }

private:
	friend class RenderViewVrfy;

	Ref< ResourceTracker > m_resourceTracker;
	Ref< IndexBuffer > m_indexBuffer;
	bool m_locked;
};

	}
}

