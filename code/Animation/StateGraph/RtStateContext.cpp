/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/StateGraph/RtStateContext.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.RtStateContext", RtStateContext, Object)

bool RtStateContext::setParameterValue(const render::Handle& handle, bool value)
{
	const auto it = m_parameters.find(handle);
	if (it != m_parameters.end())
	{
		m_values[it->second] = value;
		return true;
	}
	else
		return false;
}

bool RtStateContext::getParameterValue(const render::Handle& handle) const
{
	const auto it = m_parameters.find(handle);
	if (it != m_parameters.end())
		return m_values[it->second];
	else
		return false;
}

}
