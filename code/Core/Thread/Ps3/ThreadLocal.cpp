#include "Core/Platform.h"
#include "Core/Thread/ThreadLocal.h"

namespace traktor
{
	namespace
	{

__thread void* s_tls[16];
uint32_t s_ntls = 0;

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ThreadLocal", ThreadLocal, Object)

ThreadLocal::ThreadLocal()
:	m_handle(s_ntls++)
{
}

ThreadLocal::~ThreadLocal()
{
}

void ThreadLocal::set(void* ptr)
{
	s_tls[m_handle] = ptr;
}

void* ThreadLocal::get() const
{
	return s_tls[m_handle];
}

}
