#ifndef traktor_render_IndexBufferCapture_H
#define traktor_render_IndexBufferCapture_H

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

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void* lock() T_OVERRIDE T_FINAL;
	
	virtual void unlock() T_OVERRIDE T_FINAL;

	IndexBuffer* getIndexBuffer() const { return m_indexBuffer; }

private:
	Ref< IndexBuffer > m_indexBuffer;
	bool m_locked;
};
	
	}
}

#endif	// traktor_render_IndexBufferCapture_H
