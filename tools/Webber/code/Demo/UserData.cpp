/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Demo/UserData.h"

namespace traktor::demo
{

void User::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"name", name);
	s >> Member< std::wstring >(L"password", password);
	s >> Member< bool >(L"admin", admin);
}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.demo.Users", 0, Users, ISerializable)

void Users::serialize(ISerializer& s)
{
	s >> MemberAlignedVector< User, MemberComposite< User > >(L"users", users);
}

Ref< Users > createSampleUsers()
{
	Ref< Users > users = new Users();
	users->users.push_back({ L"apistol", L"apistol-demo", true });
	users->users.push_back({ L"klara", L"klara-demo", false });
	return users;
}

}
