/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Demo/UserData.h"

namespace traktor::demo
{

AlignedVector< User > createSampleUsers()
{
	AlignedVector< User > users;
	users.push_back({ L"apistol", L"apistol-demo", true });
	users.push_back({ L"klara", L"klara-demo", false });
	return users;
}

}
