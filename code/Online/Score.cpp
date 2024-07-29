/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Online/Score.h"

namespace traktor::online
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.Score", Score, Object)

Score::Score(const IUser* user, int32_t score, uint32_t rank)
:	m_user(user)
,	m_score(score)
,	m_rank(rank)
{
}

}
