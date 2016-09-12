#ifndef traktor_render_VertexBufferCapture_H
#define traktor_render_VertexBufferCapture_H

#include "Render/VertexBuffer.h"

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup Capture
 */
class VertexBufferCapture : public VertexBuffer
{
	T_RTTI_CLASS;

public:
	VertexBufferCapture(VertexBuffer* vertexBuffer, uint32_t bufferSize, uint32_t vertexSize);

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void* lock() T_OVERRIDE T_FINAL;

	virtual void* lock(uint32_t vertexOffset, uint32_t vertexCount) T_OVERRIDE T_FINAL;

	virtual void unlock() T_OVERRIDE T_FINAL;

	uint32_t getVertexSize() const { return m_vertexSize; }

	VertexBuffer* getVertexBuffer() const { return m_vertexBuffer; }

protected:
	uint32_t m_vertexSize;
	Ref< VertexBuffer > m_vertexBuffer;
	bool m_locked;
};
	
	}
}

#endif	// traktor_render_VertexBufferCapture_H

