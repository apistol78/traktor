#pragma once

#include "Render/VertexBuffer.h"

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup Vrfy
 */
class VertexBufferVrfy : public VertexBuffer
{
	T_RTTI_CLASS;

public:
	explicit VertexBufferVrfy(VertexBuffer* vertexBuffer, uint32_t bufferSize, uint32_t vertexSize);

	virtual ~VertexBufferVrfy();

	virtual void destroy() override final;

	virtual void* lock() override final;

	virtual void unlock() override final;

	VertexBuffer* getVertexBuffer() const { return m_vertexBuffer; }

	uint32_t getVertexSize() const { return m_vertexSize; }

protected:
	Ref< VertexBuffer > m_vertexBuffer;
	uint32_t m_vertexSize;
	bool m_locked = false;
	uint8_t* m_device = nullptr;
	uint8_t* m_shadow = nullptr;

	void verifyGuard() const;
};

	}
}

