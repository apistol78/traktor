#include <cstring>
#include "Core/Memory/Alloc.h"
#include "Core/Misc/SafeDestroy.h"
#include "Render/Vrfy/Error.h"
#include "Render/Vrfy/StructBufferVrfy.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

constexpr int32_t c_guardBytes = 16;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.StructBufferVrfy", StructBufferVrfy, StructBuffer)

StructBufferVrfy::StructBufferVrfy(StructBuffer* structBuffer, uint32_t bufferSize, uint32_t structSize)
:	StructBuffer(bufferSize)
,	m_structBuffer(structBuffer)
,	m_structSize(structSize)
{
	m_shadow = (uint8_t*)Alloc::acquireAlign(bufferSize + 2 * c_guardBytes, 16, T_FILE_LINE);
	std::memset(m_shadow, 0, bufferSize + 2 * c_guardBytes);
}

StructBufferVrfy::~StructBufferVrfy()
{
	verifyGuard();
	verifyUntouched();
	Alloc::freeAlign(m_shadow);
}

void StructBufferVrfy::destroy()
{
	T_CAPTURE_ASSERT (m_structBuffer, L"Struct buffer already destroyed.");
	T_CAPTURE_ASSERT (!m_locked, L"Cannot destroy locked struct buffer.");
	verifyGuard();
	verifyUntouched();
	safeDestroy(m_structBuffer);
}

void* StructBufferVrfy::lock()
{
	T_CAPTURE_ASSERT (m_structBuffer, L"Struct buffer destroyed.");
	T_CAPTURE_ASSERT (!m_locked, L"Struct buffer already locked.");

	verifyGuard();
	verifyUntouched();	

	if (!m_structBuffer)
		return nullptr;

	m_device = (uint8_t*)m_structBuffer->lock();
	if (m_device)
	{
		m_locked = true;
		std::memset(m_shadow, 0, getBufferSize() + 2 * c_guardBytes);
		return m_shadow + c_guardBytes;
	}
	else
		return nullptr;
}

void StructBufferVrfy::unlock()
{
	T_CAPTURE_ASSERT(m_structBuffer, L"Struct buffer destroyed.");
	T_CAPTURE_ASSERT(m_locked, L"Struct buffer not locked.");

	verifyGuard();

	if (!m_structBuffer)
		return;

	std::memcpy(m_device, m_shadow + c_guardBytes, getBufferSize());
	std::memset(m_shadow, 0, getBufferSize() + 2 * c_guardBytes);

	m_structBuffer->unlock();
	m_locked = false;
}

void StructBufferVrfy::verifyGuard() const
{
	const uint32_t bufferSize = getBufferSize();
	for (uint32_t i = 0; i < c_guardBytes; ++i)
	{
		T_CAPTURE_ASSERT(m_shadow[i] == 0x00, L"Low guard bytes overwritten.");
		T_CAPTURE_ASSERT(m_shadow[i + c_guardBytes + bufferSize] == 0x00, L"High guard bytes overwritten.");
	}
}

void StructBufferVrfy::verifyUntouched() const
{
	const uint32_t bufferSize = getBufferSize();
	for (uint32_t i = 0; i < bufferSize; ++i)
		T_CAPTURE_ASSERT(m_shadow[i + c_guardBytes] == 0x00, L"Memory touched outside of lock/unlock region.");
}

	}
}
