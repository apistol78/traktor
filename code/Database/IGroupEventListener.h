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

namespace traktor::db
{

class Group;

/*! Group event listener.
 * \ingroup Database
 */
class IGroupEventListener
{
public:
	virtual ~IGroupEventListener() {}

	virtual void groupEventRenamed(Group* group, const std::wstring& previousName) = 0;
};

}
