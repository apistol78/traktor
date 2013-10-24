#include "Core/Thread/Mutex.h"
#include "Core/Misc/TString.h"

namespace traktor
{

Mutex::Mutex()
:	m_existing(false)
,	m_handle(0)
{
}

Mutex::Mutex(const Guid& id)
:	m_existing(false)
,	m_handle(0)
{
}

Mutex::~Mutex()
{
}

bool Mutex::wait(int32_t timeout)
{
	return true;
}

void Mutex::release()
{
}

bool Mutex::existing() const
{
	return m_existing;
}

}
