/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Animation/Animation/StateContext.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.StateContext", StateContext, Object)

StateContext::StateContext()
:	m_time(0.0f)
,	m_duration(0.0f)
,	m_indexHint(-1)
{
}

	}
}
