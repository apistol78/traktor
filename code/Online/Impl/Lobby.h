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
	virtual Ref< Result > setMetaValue(const std::wstring& key, const std::wstring& value) T_OVERRIDE T_FINAL;

	virtual bool getMetaValue(const std::wstring& key, std::wstring& outValue) const T_OVERRIDE T_FINAL;

	virtual Ref< Result > setParticipantMetaValue(const std::wstring& key, const std::wstring& value) T_OVERRIDE T_FINAL;

	virtual bool getParticipantMetaValue(const IUser* user, const std::wstring& key, std::wstring& outValue) const T_OVERRIDE T_FINAL;

	virtual Ref< Result > join() T_OVERRIDE T_FINAL;

	virtual bool leave() T_OVERRIDE T_FINAL;

	virtual bool getParticipants(RefArray< IUser >& outUsers) T_OVERRIDE T_FINAL;

	virtual uint32_t getParticipantCount() const T_OVERRIDE T_FINAL;

	virtual uint32_t getMaxParticipantCount() const T_OVERRIDE T_FINAL;

	virtual uint32_t getFriendsCount() const T_OVERRIDE T_FINAL;

	virtual bool invite(const IUser* user) T_OVERRIDE T_FINAL;

	virtual bool setOwner(const IUser* user) T_OVERRIDE T_FINAL;

	virtual const IUser* getOwner() const T_OVERRIDE T_FINAL;

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
