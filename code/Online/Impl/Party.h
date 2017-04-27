/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_online_Party_H
#define traktor_online_Party_H

#include "Online/IParty.h"

namespace traktor
{
	namespace online
	{

class IMatchMakingProvider;
class TaskQueue;
class UserCache;

class Party : public IParty
{
	T_RTTI_CLASS;

public:
	virtual Ref< Result > setMetaValue(const std::wstring& key, const std::wstring& value) T_OVERRIDE T_FINAL;

	virtual bool getMetaValue(const std::wstring& key, std::wstring& outValue) const T_OVERRIDE T_FINAL;

	virtual Ref< Result > setParticipantMetaValue(const std::wstring& key, const std::wstring& value) T_OVERRIDE T_FINAL;

	virtual bool getParticipantMetaValue(const IUser* user, const std::wstring& key, std::wstring& outValue) const T_OVERRIDE T_FINAL;

	virtual bool leave() T_OVERRIDE T_FINAL;

	virtual bool getParticipants(RefArray< IUser >& outUsers) T_OVERRIDE T_FINAL;

	virtual uint32_t getParticipantCount() const T_OVERRIDE T_FINAL;

	virtual bool invite(const IUser* user) T_OVERRIDE T_FINAL;

private:
	friend class MatchMaking;
	friend class TaskCreateParty;
	friend class TaskFindMatchingLobbies;

	Ref< IMatchMakingProvider > m_matchMakingProvider;
	Ref< UserCache > m_userCache;
	Ref< TaskQueue > m_taskQueue;
	uint64_t m_handle;

	Party(IMatchMakingProvider* matchMakingProvider, UserCache* userCache, TaskQueue* taskQueue, uint64_t handle);
};

	}
}

#endif	// traktor_online_Party_H
