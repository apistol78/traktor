/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>

namespace traktor
{

class Guid;

}

namespace traktor::db
{

class Instance;

/*! Instance event listener.
 * \ingroup Database
 */
class IInstanceEventListener
{
public:
	virtual ~IInstanceEventListener() {}

	virtual void instanceEventCreated(Instance* instance) = 0;

	virtual void instanceEventRemoved(Instance* instance) = 0;

	virtual void instanceEventGuidChanged(Instance* instance, const Guid& previousGuid) = 0;

	virtual void instanceEventRenamed(Instance* instance, const std::wstring& previousName) = 0;

	virtual void instanceEventCommitted(Instance* instance) = 0;
};

}
