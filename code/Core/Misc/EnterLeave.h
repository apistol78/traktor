/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <functional>

namespace traktor
{

/*! Automatically call functors when entering and leaving scope.
 * \ingroup Core
 */
class EnterLeave
{
public:
	explicit EnterLeave(const std::function< void() >& enter, const std::function< void() >& leave)
	:	m_leave(leave)
	{
		if (enter)
			enter();
	}

	inline ~EnterLeave()
	{
		if (m_leave)
			m_leave();
	}

private:
	std::function< void() > m_leave;
};

/*! Automatically call functors when leaving scope.
 * \ingroup Core
 */
class Leave
{
public:
	explicit Leave(const std::function< void() >& leave)
	:	m_leave(leave)
	{
	}

	inline ~Leave()
	{
		if (m_leave)
			m_leave();
	}

private:
	std::function< void() > m_leave;
};

}
