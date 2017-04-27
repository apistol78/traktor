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
	namespace
	{

__thread void* s_tls[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
__thread bool s_tlsOccupied[16] = { false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false };

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ThreadLocal", ThreadLocal, Object)

ThreadLocal::ThreadLocal()
:	m_handle(0)
{
	for (uint32_t i = 0; i < sizeof_array(s_tls); ++i)
	{
		if (!s_tlsOccupied[i])
		{
			s_tls[i] = 0;
			s_tlsOccupied[i] = true;
			m_handle = i;
			break;
		}
	}
}

ThreadLocal::~ThreadLocal()
{
	s_tlsOccupied[m_handle] = false;
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
