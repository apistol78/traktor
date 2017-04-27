/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Platform.h"
#include "Core/Thread/Mutex.h"
#include "Core/Misc/TString.h"

namespace traktor
{

Mutex::Mutex()
:	m_existing(false)
{
	m_handle = CreateMutex(0, FALSE, NULL);
	T_ASSERT (m_handle != NULL);
}

Mutex::Mutex(const Guid& id)
{
	m_handle = CreateMutex(
		0,
		FALSE,
		wstots(L"Global\\" + id.format()).c_str()
	);
	m_existing = bool(GetLastError() == ERROR_ALREADY_EXISTS);
	T_ASSERT (m_handle != NULL);
}

Mutex::~Mutex()
{
	CloseHandle(m_handle);
}

bool Mutex::wait(int32_t timeout)
{
#if !defined(_XBOX)
	MMRESULT result = timeBeginPeriod(1);
#endif

	bool ret = bool(WaitForSingleObject(m_handle, (timeout < 0) ? INFINITE : timeout) == WAIT_OBJECT_0);

#if !defined(_XBOX)
	if (result == TIMERR_NOERROR)
		timeEndPeriod(1);
#endif

	return ret;
}

void Mutex::release()
{
	ReleaseMutex(m_handle);
}

bool Mutex::existing() const
{
	return m_existing;
}

}
