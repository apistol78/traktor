/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Platform.h"
#include "Core/Thread/CriticalSection.h"

namespace traktor
{

CriticalSection::CriticalSection()
{
	m_handle = new CRITICAL_SECTION;
	InitializeCriticalSection((LPCRITICAL_SECTION)m_handle);
}

CriticalSection::~CriticalSection()
{
	DeleteCriticalSection((LPCRITICAL_SECTION)m_handle);
	delete static_cast< CRITICAL_SECTION* >(m_handle);
}

bool CriticalSection::wait(int32_t timeout)
{
	T_ASSERT_M (timeout == -1, L"Invalid timeout");
	EnterCriticalSection((LPCRITICAL_SECTION)m_handle);
	return true;
}

void CriticalSection::release()
{
	LeaveCriticalSection((LPCRITICAL_SECTION)m_handle);
}

}
