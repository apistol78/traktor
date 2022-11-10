/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"

namespace traktor
{
	namespace db
	{

class Context;

/*! Transaction action.
 * \ingroup Database
 */
class Action : public Object
{
	T_RTTI_CLASS;

public:
	virtual bool execute(Context& context) = 0;

	virtual bool undo(Context& context) = 0;

	virtual void clean(Context& context) = 0;

	virtual bool redundant(const Action* action) const = 0;
};

	}
}

