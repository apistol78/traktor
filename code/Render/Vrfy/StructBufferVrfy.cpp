#include "Core/Misc/SafeDestroy.h"
#include "Render/Vrfy/Error.h"
#include "Render/Vrfy/StructBufferVrfy.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.StructBufferVrfy", StructBufferVrfy, StructBuffer)

StructBufferVrfy::StructBufferVrfy(StructBuffer* structBuffer, uint32_t bufferSize, uint32_t structSize)
:	StructBuffer(bufferSize)
,	m_structBuffer(structBuffer)
,	m_structSize(structSize)
,	m_locked(false)
{
}

void StructBufferVrfy::destroy()
{
	T_CAPTURE_ASSERT (m_structBuffer, L"Struct buffer already destroyed.");
	T_CAPTURE_ASSERT (!m_locked, L"Cannot destroy locked struct buffer.");
	safeDestroy(m_structBuffer);
}

void* StructBufferVrfy::lock()
{
	T_CAPTURE_ASSERT (m_structBuffer, L"Struct buffer destroyed.");
	T_CAPTURE_ASSERT (!m_locked, L"Struct buffer already locked.");

	if (!m_structBuffer)
		return nullptr;

	void* p = m_structBuffer->lock();
	if (p)
		m_locked = true;

	return p;
}

void* StructBufferVrfy::lock(uint32_t structOffset, uint32_t structCount)
{
	T_CAPTURE_ASSERT (m_structBuffer, L"Struct buffer destroyed.");
	T_CAPTURE_ASSERT (!m_locked, L"Struct buffer already locked.");
	T_CAPTURE_ASSERT (structOffset + structCount <= getBufferSize() / m_structSize, L"Trying to lock struct buffer out of range.");

	if (!m_structBuffer)
		return nullptr;

	void* p = m_structBuffer->lock(structOffset, structCount);
	if (p)
		m_locked = true;

	return p;
}

void StructBufferVrfy::unlock()
{
	T_CAPTURE_ASSERT (m_structBuffer, L"Struct buffer destroyed.");
	T_CAPTURE_ASSERT (m_locked, L"Struct buffer not locked.");

	if (!m_structBuffer)
		return;

	m_structBuffer->unlock();
	m_locked = false;
}

	}
}
