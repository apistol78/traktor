#if defined(__EMSCRIPTEN_PTHREADS__)
#	include <pthread.h>
#endif
#include "Core/Platform.h"
#include "Core/Thread/ThreadLocal.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ThreadLocal", ThreadLocal, Object)

ThreadLocal::ThreadLocal()
{
#if defined(__EMSCRIPTEN_PTHREADS__)
	pthread_key_t& key = *reinterpret_cast< pthread_key_t* >(&m_handle);
	int32_t rc = pthread_key_create(&key, 0);
	T_FATAL_ASSERT_M(rc == 0, L"Unable to create thread key");
#endif
}

ThreadLocal::~ThreadLocal()
{
#if defined(__EMSCRIPTEN_PTHREADS__)
	pthread_key_t& key = *reinterpret_cast< pthread_key_t* >(&m_handle);
	pthread_key_delete(key);
#endif
}

void ThreadLocal::set(void* ptr)
{
#if defined(__EMSCRIPTEN_PTHREADS__)
	pthread_key_t& key = *reinterpret_cast< pthread_key_t* >(&m_handle);
	pthread_setspecific(key, ptr);
#else
	*(void**)&m_handle = ptr;
#endif
}

void* ThreadLocal::get() const
{
#if defined(__EMSCRIPTEN_PTHREADS__)
	const pthread_key_t& key = *reinterpret_cast< const pthread_key_t* >(&m_handle);
	return pthread_getspecific(key);
#else
	return (void*)m_handle;
#endif
}

}
