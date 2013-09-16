#ifndef traktor_online_Lobby_H
#define traktor_online_Lobby_H

#include "Online/ILobby.h"

namespace traktor
{
	namespace online
	{

class IMatchMakingProvider;
class TaskQueue;
class UserCache;

class Lobby : public ILobby
{
	T_RTTI_CLASS;

public:
	virtual Ref< Result > setMetaValue(const std::wstring& key, const std::wstring& value);

	virtual bool getMetaValue(const std::wstring& key, std::wstring& outValue) const;

	virtual Ref< Result > setParticipantMetaValue(const std::wstring& key, const std::wstring& value);

	virtual bool getParticipantMetaValue(const IUser* user, const std::wstring& key, std::wstring& outValue) const;

	virtual Ref< Result > join();

	virtual bool leave();

	virtual RefArray< IUser > getParticipants();

	virtual uint32_t getParticipantCount() const;

	virtual uint32_t getMaxParticipantCount() const;

	virtual uint32_t getFriendsCount() const;

	virtual bool invite(const IUser* user);

	virtual int32_t getIndex() const;

	virtual bool setOwner(const IUser* user);

	virtual const IUser* getOwner() const;

private:
	friend class MatchMaking;
	friend class TaskCreateLobby;
	friend class TaskFindMatchingLobbies;

	Ref< IMatchMakingProvider > m_matchMakingProvider;
	Ref< UserCache > m_userCache;
	Ref< TaskQueue > m_taskQueue;
	uint64_t m_handle;

	Lobby(IMatchMakingProvider* matchMakingProvider, UserCache* userCache, TaskQueue* taskQueue, uint64_t handle);
};

	}
}

#endif	// traktor_online_Lobby_H
