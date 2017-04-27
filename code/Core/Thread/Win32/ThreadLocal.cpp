/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Platform.h"
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
	TlsFree(m_handle);
}

void ThreadLocal::set(void* ptr)
{
	TlsSetValue(m_handle, ptr);
}

void* ThreadLocal::get() const
{
	return TlsGetValue(m_handle);
}

}
