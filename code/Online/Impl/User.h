/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Online/IUser.h"

namespace traktor
{
	namespace online
	{

class IUserProvider;

class User : public IUser
{
	T_RTTI_CLASS;

public:
	virtual bool getName(std::wstring& outName) const override final;

	virtual Ref< drawing::Image > getImage() const override final;

	virtual uint64_t getGlobalId() const override final;

	virtual bool isFriend() const override final;

	virtual bool isMemberOfGroup(const std::wstring& groupName) const override final;

	virtual bool joinGroup(const std::wstring& groupName) override final;

	virtual bool invite() override final;

	virtual bool setPresenceValue(const std::wstring& key, const std::wstring& value) override final;

	virtual bool getPresenceValue(const std::wstring& key, std::wstring& outValue) const override final;

	virtual void setP2PEnable(bool enable) override final;

	virtual bool isP2PAllowed() const override final;

	virtual bool isP2PRelayed() const override final;

	virtual bool sendP2PData(const void* data, size_t size, bool reliable) override final;

private:
	friend class Lobby;
	friend class Party;
	friend class UserCache;
	friend class VoiceChat;

	Ref< IUserProvider > m_userProvider;
	uint64_t m_handle;

	User(IUserProvider* userProvider, uint64_t handle);
};

	}
}

