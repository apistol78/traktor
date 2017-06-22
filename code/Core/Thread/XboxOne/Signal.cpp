/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Thread/Signal.h"

namespace traktor
{

Signal::Signal()
{
	m_handle = 0;
}

Signal::~Signal()
{
}

void Signal::set()
{
}

void Signal::reset()
{
}

bool Signal::wait(int timeout)
{
	return true;
}

}
