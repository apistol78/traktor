/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"

namespace traktor
{

/*! Safe destroy call.
 *
 * Helper function to reduce code bloat
 * in common pattern.
 */
template < typename T >
void safeDestroy(T& tv)
{
	if (tv)
	{
		T ttv = tv; tv = nullptr;
		ttv->destroy();
	}
}

/*! Safe destroy call.
 *
 * Helper function to reduce code bloat
 * in common pattern.
 */
template < typename T >
void safeDestroy(RefArray< T >& tvs)
{
	for (auto tv : tvs)
	{
		if (tv)
			tv->destroy();
	}
	tvs.clear();
}

/*! Safe close call.
 *
 * Helper function to reduce code bloat
 * in common pattern.
 */
template < typename T >
void safeClose(T& tv)
{
	if (tv)
	{
		T ttv = tv; tv = nullptr;
		ttv->close();
	}
}

}

