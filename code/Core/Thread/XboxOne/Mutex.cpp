/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Thread/Mutex.h"

namespace traktor
{

Mutex::Mutex()
:	m_existing(false)
{
	m_handle = 0;
}

Mutex::Mutex(const Guid& id)
{
	m_handle = 0;
	m_existing = false;
}

Mutex::~Mutex()
{
}

bool Mutex::wait(int32_t timeout)
{
	return true;
}

void Mutex::release()
{
}

bool Mutex::existing() const
{
	return m_existing;
}

}
