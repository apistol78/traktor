/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

namespace traktor
{

/*! \ingroup Core */
//@{

/*! Type equal check. */
template <
	typename Type1,
	typename Type2
>
struct Equal
{
	enum { value = false };
};

/*! Type equal check, matching type specialization. */
template <
	typename Type
>
struct Equal < Type, Type >
{
	enum { value = true };
};

//@}

}

