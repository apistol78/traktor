#include "Core/Thread/CriticalSection.h"

namespace traktor
{

CriticalSection::CriticalSection()
{
}

CriticalSection::~CriticalSection()
{
}

bool CriticalSection::wait(int32_t timeout)
{
	return true;
}

void CriticalSection::release()
{
}

}
