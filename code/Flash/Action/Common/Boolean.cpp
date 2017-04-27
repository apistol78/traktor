/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Flash/Action/ActionValue.h"
#include "Flash/Action/Common/Boolean.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.Boolean", Boolean, ActionObjectRelay)

Boolean::Boolean(bool value)
:	ActionObjectRelay("Boolean")
,	m_value(value)
{
}

	}
}
