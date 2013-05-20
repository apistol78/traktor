#include "Core/Platform.h"
#include "Core/Thread/ThreadLocal.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ThreadLocal", ThreadLocal, Object)

ThreadLocal::ThreadLocal()
{
}

ThreadLocal::~ThreadLocal()
{
}

void ThreadLocal::set(void* ptr)
{
}

void* ThreadLocal::get() const
{
	return 0;
}

}
