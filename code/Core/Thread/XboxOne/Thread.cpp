/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Thread/Thread.h"

namespace traktor
{

Thread::Thread(Functor* functor, const std::wstring& name, int32_t hardwareCore)
:	m_handle(0)
,	m_id(0)
,	m_stopped(false)
{
}

Thread::~Thread()
{
}

bool Thread::start(Priority priority)
{
	return false;
}

bool Thread::wait(int32_t timeout)
{
	return true;
}

bool Thread::stop(int32_t timeout)
{
	return true;
}

bool Thread::pause()
{
	return false;
}

bool Thread::resume()
{
	return false;
}

void Thread::sleep(int duration)
{
}

void Thread::yield()
{
}

bool Thread::stopped() const
{
	return false;
}

bool Thread::current() const
{
	return false;
}

bool Thread::finished() const
{
	return false;
}

}
