/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Online/Impl/Party.h"
#include "Online/Impl/TaskQueue.h"
#include "Online/Impl/User.h"
#include "Online/Impl/UserCache.h"
//#include "Online/Impl/Tasks/TaskSetPartyMetaValue.h"
//#include "Online/Impl/Tasks/TaskSetPartyParticipantMetaValue.h"
#include "Online/Provider/IMatchMakingProvider.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.Party", Party, IParty)

Ref< Result > Party::setMetaValue(const std::wstring& key, const std::wstring& value)
{
	//if (!m_matchMakingProvider)
	//	return 0;

	//Ref< Result > result = new Result();
	//if (m_taskQueue->add(new TaskSetPartyMetaValue(
	//	m_matchMakingProvider,
	//	m_handle,
	//	key,
	//	value,
	//	result
	//)))
	//	return result;
	//else
	//	return 0;

	return 0;
}

bool Party::getMetaValue(const std::wstring& key, std::wstring& outValue) const
{
	return m_matchMakingProvider ? m_matchMakingProvider->getPartyMetaValue(m_handle, key, outValue) : false;
}

Ref< Result > Party::setParticipantMetaValue(const std::wstring& key, const std::wstring& value)
{
	//if (!m_matchMakingProvider)
	//	return 0;

	//Ref< Result > result = new Result();
	//if (m_taskQueue->add(new TaskSetPartyParticipantMetaValue(
	//	m_matchMakingProvider,
	//	m_handle,
	//	key,
	//	value,
	//	result
	//)))
	//	return result;
	//else
	//	return 0;

	return 0;
}

bool Party::getParticipantMetaValue(const IUser* user, const std::wstring& key, std::wstring& outValue) const
{
	if (!m_matchMakingProvider)
		return false;

	const User* userImpl = dynamic_type_cast< const User* >(user);
	if (userImpl)
		return m_matchMakingProvider->getPartyParticipantMetaValue(m_handle, userImpl->m_handle, key, outValue);
	else
		return false;
}

bool Party::leave()
{
	if (!m_matchMakingProvider)
		return true;

	if (!m_matchMakingProvider->leaveParty(m_handle))
		return false;

	m_matchMakingProvider = 0;
	return true;
}

bool Party::getParticipants(RefArray< IUser >& outUsers)
{
	if (!m_matchMakingProvider)
		return false;

	std::vector< uint64_t > userHandles;
	userHandles.reserve(64);

	m_matchMakingProvider->getPartyParticipants(m_handle, userHandles);
	m_userCache->getMany(userHandles, (RefArray< User >&)outUsers);

	return true;
}

uint32_t Party::getParticipantCount() const
{
	uint32_t count;
	if (m_matchMakingProvider && m_matchMakingProvider->getPartyParticipantCount(m_handle, count))
		return count;
	else
		return 0;
}

bool Party::invite(const IUser* user)
{
	const User* userImpl = dynamic_type_cast< const User* >(user);
	if (!userImpl)
		return false;

	if (m_matchMakingProvider && m_matchMakingProvider->inviteToParty(m_handle, userImpl->m_handle))
		return true;
	else
		return true;
}

Party::Party(IMatchMakingProvider* matchMakingProvider, UserCache* userCache, TaskQueue* taskQueue, uint64_t handle)
:	m_matchMakingProvider(matchMakingProvider)
,	m_userCache(userCache)
,	m_taskQueue(taskQueue)
,	m_handle(handle)
{
}

	}
}
