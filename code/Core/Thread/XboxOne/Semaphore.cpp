/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Thread/Semaphore.h"

namespace traktor
{

Semaphore::Semaphore()
{
	m_handle = 0;
}

Semaphore::~Semaphore()
{
}

bool Semaphore::wait(int32_t timeout)
{
	return true;
}

void Semaphore::release()
{
}

}
