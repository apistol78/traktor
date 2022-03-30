#include "Core/System.h"
#include "Core/Thread/ThreadLocal.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ThreadLocal", ThreadLocal, Object)

ThreadLocal::ThreadLocal()
{
	m_handle = TlsAlloc();
}

ThreadLocal::~ThreadLocal()
{
	TlsFree((DWORD)m_handle);
}

void ThreadLocal::set(void* ptr)
{
	TlsSetValue((DWORD)m_handle, ptr);
}

void* ThreadLocal::get() const
{
	return TlsGetValue((DWORD)m_handle);
}

}
