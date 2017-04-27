/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <algorithm>
#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Core/Timer/LimitScope.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.LimitScope", LimitScope, Object)

LimitScope::LimitScope(const char* name, float threshold, Context& context)
:	m_name(name)
,	m_threshold(threshold)
,	m_context(context)
{
	m_timer.start();
}

LimitScope::~LimitScope()
{
	float elapsed = float(m_timer.getElapsedTime() * 1000.0f);
	float result = elapsed - m_threshold;
	if (result > 0.0f)
	{
		m_context.m_peek = std::max(m_context.m_peek, elapsed);
		m_context.m_count++;
		T_DEBUG(L"Time limit \"" << mbstows(m_name) << L"\" exceeded by " << result << L" ms (limit " << m_threshold << L" ms, current " << elapsed << L" ms, peek " << m_context.m_peek << L" ms) " << m_context.m_count << L" time(s)");
	}
}

}
