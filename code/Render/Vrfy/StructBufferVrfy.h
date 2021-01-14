#ifndef traktor_render_StructBufferVrfy_H
#define traktor_render_StructBufferVrfy_H

#include "Render/StructBuffer.h"

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup Vrfy
 */
class StructBufferVrfy : public StructBuffer
{
	T_RTTI_CLASS;

public:
	StructBufferVrfy(StructBuffer* structBuffer, uint32_t bufferSize, uint32_t structSize);

	virtual void destroy() override final;

	virtual void* lock() override final;

	virtual void* lock(uint32_t vertexOffset, uint32_t vertexCount) override final;

	virtual void unlock() override final;

	StructBuffer* getStructBuffer() const { return m_structBuffer; }

	uint32_t getStructSize() const { return m_structSize; }

protected:
	Ref< StructBuffer > m_structBuffer;
	uint32_t m_structSize;
	bool m_locked;
};

	}
}

#endif	// traktor_render_StructBufferVrfy_H
