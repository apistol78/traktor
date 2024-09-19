/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/Murmur3.h"
#include "Core/Test/CaseMurmur.h"

namespace traktor::test
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.test.CaseMurmur", 0, CaseMurmur, Case)

void CaseMurmur::run()
{
	const char text[] = "Hello world! Testing Murmur3 hash.";

	Murmur3 m;
	m.begin();
	m.feedBuffer(text, sizeof(text));
	m.end();

	const uint32_t h = m.get();
	CASE_ASSERT(h == 3720714118);
}

}
