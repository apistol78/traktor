/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Webber/Event.h"

namespace traktor::wbr
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.wbr.Event", Event, Object)

void Event::raise()
{
	// Iterate a copy; handlers may add handlers.
	const auto fns = m_fns;
	for (const auto& fn : fns)
		fn();
}

void Event::operator += (const std::function< void() >& fn)
{
	m_fns.push_back(fn);
}

}
