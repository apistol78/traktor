/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Flash/Action/ActionValue.h"
#include "Flash/Action/Common/String.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.String", String, ActionObjectRelay)

String::String()
:	ActionObjectRelay("String")
{
}

String::String(char ch)
:	ActionObjectRelay("String")
{
	m_str.resize(1, ch);
}

String::String(const std::string& str)
:	ActionObjectRelay("String")
,	m_str(str)
{
}

	}
}
