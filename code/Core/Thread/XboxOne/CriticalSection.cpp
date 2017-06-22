/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Thread/CriticalSection.h"

namespace traktor
{

CriticalSection::CriticalSection()
{
	m_handle = 0;
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
