/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/LogStreamTarget.h"
#include "Core/Misc/String.h"

namespace traktor
{

LogStreamTarget::LogStreamTarget(OutputStream* stream)
:	m_stream(stream)
{
}

void LogStreamTarget::log(uint32_t threadId, int32_t level, const wchar_t* str)
{
	(*m_stream) << traktor::str(L"[%5d] ", threadId) << str << Endl;
}

}
