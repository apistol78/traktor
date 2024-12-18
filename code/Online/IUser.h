/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Object.h"
#include "Core/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ONLINE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::drawing
{

class Image;

}

namespace traktor::online
{

class T_DLLCLASS IUser : public Object
{
	T_RTTI_CLASS;

public:
	virtual bool getName(std::wstring& outName) const = 0;

	virtual Ref< drawing::Image > getImage() const = 0;

	virtual uint64_t getGlobalId() const = 0;

	virtual bool isFriend() const = 0;

	virtual bool isMemberOfGroup(const std::wstring& groupName) const = 0;

	virtual bool joinGroup(const std::wstring& groupName) = 0;

	virtual bool invite() = 0;

	virtual bool setPresenceValue(const std::wstring& key, const std::wstring& value) = 0;

	virtual bool getPresenceValue(const std::wstring& key, std::wstring& outValue) const = 0;

	virtual void setP2PEnable(bool enable) = 0;

	virtual bool isP2PAllowed() const = 0;

	virtual bool isP2PRelayed() const = 0;

	virtual bool sendP2PData(const void* data, size_t size, bool reliable) = 0;
};

}
