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
#include "Core/Ref.h"
#include "Core/Serialization/ISerializable.h"

namespace traktor
{

class ISerializer;

}

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

	void serialize(ISerializer& s);
};

/*! User accounts; root of the users data file.
 * \ingroup Webber
 */
class Users : public ISerializable
{
	T_RTTI_CLASS;

public:
	AlignedVector< User > users;

	virtual void serialize(ISerializer& s) override final;
};

/*! Create the demo's user accounts. */
Ref< Users > createSampleUsers();

}
