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
	virtual bool getName(std::wstring& outName) const;

	virtual Ref< drawing::Image > getImage() const;

	virtual uint64_t getGlobalId() const;

	virtual bool isFriend() const;

	virtual bool isMemberOfGroup(const std::wstring& groupName) const;

	virtual bool joinGroup(const std::wstring& groupName);

	virtual bool invite();

	virtual bool setPresenceValue(const std::wstring& key, const std::wstring& value);

	virtual bool getPresenceValue(const std::wstring& key, std::wstring& outValue) const;

	virtual bool isP2PAllowed() const;

	virtual bool isP2PRelayed() const;

	virtual bool sendP2PData(const void* data, size_t size, bool reliable);

private:
	friend class Lobby;
	friend class Party;
	friend class UserCache;

	Ref< IUserProvider > m_userProvider;
	uint64_t m_handle;

	User(IUserProvider* userProvider, uint64_t handle);
};

	}
}

#endif	// traktor_online_User_H
