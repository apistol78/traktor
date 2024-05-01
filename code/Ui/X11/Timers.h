/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <functional>
#include <map>

namespace traktor::ui
{

class Timers
{
public:
	static Timers& getInstance();

	int32_t bind(int32_t interval, const std::function< void() >& fn);

	void unbind(int32_t id);

	void update(double s);

private:
	struct Timer
	{
		int32_t interval;
		double until;
		std::function< void() > fn;
	};

	std::map< int32_t, Timer > m_timers;
	int32_t m_nid = 1;
};

}
