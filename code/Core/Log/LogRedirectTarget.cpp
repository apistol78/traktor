/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/LogRedirectTarget.h"

namespace traktor
{

LogRedirectTarget::LogRedirectTarget(ILogTarget* target)
{
	m_targets.push_back(target);
}

LogRedirectTarget::LogRedirectTarget(ILogTarget* target1, ILogTarget* target2)
{
	m_targets.push_back(target1);
	m_targets.push_back(target2);
}

void LogRedirectTarget::log(uint32_t threadId, int32_t level, const wchar_t* str)
{
	for (auto target : m_targets)
		target->log(threadId, level, str);
}

}
