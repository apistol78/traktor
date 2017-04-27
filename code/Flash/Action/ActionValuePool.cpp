/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Flash/Action/ActionValue.h"
#include "Flash/Action/ActionValuePool.h"

namespace traktor
{
	namespace flash
	{

ActionValuePool::ActionValuePool()
:	m_next(0)
{
	m_top.reset(new ActionValue [Capacity]);
	T_FATAL_ASSERT_M (m_top.ptr(), L"Out of memory");
}

	}
}
