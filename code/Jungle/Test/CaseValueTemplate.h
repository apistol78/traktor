/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Test/Case.h"

namespace traktor::jungle::test
{

class CaseValueTemplate : public traktor::test::Case
{
	T_RTTI_CLASS;

public:
	virtual void run() override final;
};

}
