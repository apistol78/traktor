/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <functional>
#include <vector>
#include "Core/Object.h"

namespace traktor::wbr
{

/*! Widget event; calls all added handlers when raised.
 * \ingroup Webber
 */
class Event : public Object
{
	T_RTTI_CLASS;

public:
	void raise();

	void operator += (const std::function< void() >& fn);

private:
	std::vector< std::function< void() > > m_fns;
};

}
