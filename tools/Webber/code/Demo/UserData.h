/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Containers/AlignedVector.h"

namespace traktor::demo
{

/*! User account.
 * \ingroup Webber
 */
struct User
{
	std::wstring name;
	std::wstring password;
	bool admin;
};

/*! Create the demo's user accounts. */
AlignedVector< User > createSampleUsers();

}
