#ifndef traktor_online_ILobby_H
#define traktor_online_ILobby_H

#include "Online/UserArrayResult.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ONLINE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace online
	{

class IUser;

class T_DLLCLASS ILobby : public Object
{
	T_RTTI_CLASS;

public:
	virtual Ref< Result > setMetaValue(const std::wstring& key, const std::wstring& value) = 0;

	virtual bool getMetaValue(const std::wstring& key, std::wstring& outValue) const = 0;

	virtual Ref< Result > setParticipantMetaValue(const std::wstring& key, const std::wstring& value) = 0;

	virtual bool getParticipantMetaValue(const IUser* user, const std::wstring& key, std::wstring& outValue) const = 0;

	virtual Ref< Result > join() = 0;

	virtual bool leave() = 0;

	virtual RefArray< IUser > getParticipants() = 0;

	virtual uint32_t getParticipantCount() const = 0;

	virtual uint32_t getMaxParticipantCount() const = 0;

	virtual uint32_t getFriendsCount() const = 0;

	virtual bool invite(const IUser* user) = 0;

	virtual int32_t getIndex() const = 0;

	virtual bool setOwner(const IUser* user) = 0;

	virtual const IUser* getOwner() const = 0;
};

	}
}

#endif	// traktor_online_ILobby_H
