/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <vector>
#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Thread/Semaphore.h"

namespace traktor
{
	namespace online
	{

class IUserProvider;
class User;

class UserCache : public Object
{
	T_RTTI_CLASS;

public:
	UserCache(IUserProvider* userProvider);

	User* get(uint64_t userHandle);

	void getMany(const std::vector< uint64_t >& userHandles, RefArray< User >& outUsers);

private:
	Semaphore m_lock;
	Ref< IUserProvider > m_userProvider;
	SmallMap< uint64_t, Ref< User > > m_users;
};

	}
}

