/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_online_User_H
#define traktor_online_User_H

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
	virtual bool getName(std::wstring& outName) const T_OVERRIDE T_FINAL;

	virtual Ref< drawing::Image > getImage() const T_OVERRIDE T_FINAL;

	virtual uint64_t getGlobalId() const T_OVERRIDE T_FINAL;

	virtual bool isFriend() const T_OVERRIDE T_FINAL;

	virtual bool isMemberOfGroup(const std::wstring& groupName) const T_OVERRIDE T_FINAL;

	virtual bool joinGroup(const std::wstring& groupName) T_OVERRIDE T_FINAL;

	virtual bool invite() T_OVERRIDE T_FINAL;

	virtual bool setPresenceValue(const std::wstring& key, const std::wstring& value) T_OVERRIDE T_FINAL;

	virtual bool getPresenceValue(const std::wstring& key, std::wstring& outValue) const T_OVERRIDE T_FINAL;

	virtual void setP2PEnable(bool enable) T_OVERRIDE T_FINAL;

	virtual bool isP2PAllowed() const T_OVERRIDE T_FINAL;

	virtual bool isP2PRelayed() const T_OVERRIDE T_FINAL;

	virtual bool sendP2PData(const void* data, size_t size, bool reliable) T_OVERRIDE T_FINAL;

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

#endif	// traktor_online_User_H
