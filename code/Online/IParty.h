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
#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/RefArray.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ONLINE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Result;

	namespace online
	{

class IUser;

class T_DLLCLASS IParty : public Object
{
	T_RTTI_CLASS;

public:
	virtual Ref< Result > setMetaValue(const std::wstring& key, const std::wstring& value) = 0;

	virtual bool getMetaValue(const std::wstring& key, std::wstring& outValue) const = 0;

	virtual Ref< Result > setParticipantMetaValue(const std::wstring& key, const std::wstring& value) = 0;

	virtual bool getParticipantMetaValue(const IUser* user, const std::wstring& key, std::wstring& outValue) const = 0;

	virtual bool leave() = 0;

	virtual bool getParticipants(RefArray< IUser >& outUsers) = 0;

	virtual uint32_t getParticipantCount() const = 0;

	virtual bool invite(const IUser* user) = 0;
};

	}
}

