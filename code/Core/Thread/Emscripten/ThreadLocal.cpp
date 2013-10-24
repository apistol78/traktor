#include "Core/Thread/ThreadLocal.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ThreadLocal", ThreadLocal, Object)

ThreadLocal::ThreadLocal()
{
	void*& key = *reinterpret_cast< void** >(&m_handle);
	key = 0;
}

ThreadLocal::~ThreadLocal()
{
}

void ThreadLocal::set(void* ptr)
{
	void*& key = *reinterpret_cast< void** >(&m_handle);
	key = ptr;
}

void* ThreadLocal::get() const
{
	return *(void**)(&m_handle);
}

}
