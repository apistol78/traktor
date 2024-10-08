/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Online/Gc/GcUser.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.GcUser", GcUser, IUserProvider)

bool GcUser::getName(uint64_t userHandle, std::wstring& outName)
{
	return false;
}

Ref< drawing::Image > GcUser::getImage(uint64_t userHandle) const
{
	return 0;
}

bool GcUser::isFriend(uint64_t userHandle)
{
	return false;
}

bool GcUser::isMemberOfGroup(uint64_t userHandle, const std::wstring& groupName) const
{
	return false;
}

bool GcUser::joinGroup(uint64_t userHandle, const std::wstring& groupName)
{
	return false;
}

bool GcUser::invite(uint64_t userHandle)
{
	return false;
}

bool GcUser::setPresenceValue(uint64_t userHandle, const std::wstring& key, const std::wstring& value)
{
	return false;
}

bool GcUser::getPresenceValue(uint64_t userHandle, const std::wstring& key, std::wstring& outValue)
{
	return false;
}

void GcUser::setP2PEnable(uint64_t userHandle, bool enable)
{
}

bool GcUser::isP2PAllowed(uint64_t userHandle) const
{
	return false;
}

bool GcUser::isP2PRelayed(uint64_t userHandle) const
{
	return false;
}

bool GcUser::sendP2PData(uint64_t userHandle, const void* data, size_t size, bool reliable)
{
	return false;
}

	}
}
