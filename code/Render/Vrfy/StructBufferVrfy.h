#pragma once

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
	explicit StructBufferVrfy(StructBuffer* structBuffer, uint32_t bufferSize, uint32_t structSize);

	virtual ~StructBufferVrfy();

	virtual void destroy() override final;

	virtual void* lock() override final;

	virtual void unlock() override final;

	StructBuffer* getStructBuffer() const { return m_structBuffer; }

	uint32_t getStructSize() const { return m_structSize; }

protected:
	Ref< StructBuffer > m_structBuffer;
	uint32_t m_structSize;
	bool m_locked = false;
	uint8_t* m_device = nullptr;
	uint8_t* m_shadow = nullptr;
	void* m_callstack[8] = { 0 };

	void verifyGuard() const;

	void verifyUntouched() const;
};

	}
}

