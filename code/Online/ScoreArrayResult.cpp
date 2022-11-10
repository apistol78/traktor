/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Online/ScoreArrayResult.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.ScoreArrayResult", ScoreArrayResult, Result)

void ScoreArrayResult::succeed(const RefArray< Score >& score)
{
	m_score = score;
	Result::succeed();
}

const RefArray< Score >& ScoreArrayResult::get() const
{
	wait();
	return m_score;
}

	}
}
