/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Config.h"

namespace traktor
{

/*! Managed object reference debugger interface.
 * \ingroup Core
 */
struct T_NOVTABLE IObjectRefDebugger
{
	virtual ~IObjectRefDebugger() {}

	virtual void addObject(void* object, size_t size) = 0;

	virtual void removeObject(void* object) = 0;

	virtual void addObjectRef(void* ref, void* object) = 0;

	virtual void removeObjectRef(void* ref, void* object) = 0;
};

}

