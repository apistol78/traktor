#include "Online/Impl/Lobby.h"
#include "Online/Impl/TaskQueue.h"
#include "Online/Impl/User.h"
#include "Online/Impl/UserCache.h"
#include "Online/Impl/Tasks/TaskJoinLobby.h"
#include "Online/Impl/Tasks/TaskSetLobbyMetaValue.h"
#include "Online/Impl/Tasks/TaskSetLobbyParticipantMetaValue.h"
#include "Online/Provider/IMatchMakingProvider.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.Lobby", Lobby, ILobby)

Ref< Result > Lobby::setMetaValue(const std::wstring& key, const std::wstring& value)
{
	if (!m_matchMakingProvider)
		return 0;

	Ref< Result > result = new Result();
	if (m_taskQueue->add(new TaskSetLobbyMetaValue(
		m_matchMakingProvider,
		m_handle,
		key,
		value,
		result
	)))
		return result;
	else
		return 0;
}

bool Lobby::getMetaValue(const std::wstring& key, std::wstring& outValue) const
{
	return m_matchMakingProvider ? m_matchMakingProvider->getLobbyMetaValue(m_handle, key, outValue) : false;
}

Ref< Result > Lobby::setParticipantMetaValue(const std::wstring& key, const std::wstring& value)
{
	if (!m_matchMakingProvider)
		return 0;

	Ref< Result > result = new Result();
	if (m_taskQueue->add(new TaskSetLobbyParticipantMetaValue(
		m_matchMakingProvider,
		m_handle,
		key,
		value,
		result
	)))
		return result;
	else
		return 0;
}

bool Lobby::getParticipantMetaValue(const IUser* user, const std::wstring& key, std::wstring& outValue) const
{
	if (!m_matchMakingProvider)
		return false;

	const User* userImpl = dynamic_type_cast< const User* >(user);
	if (userImpl)
		return m_matchMakingProvider->getLobbyParticipantMetaValue(m_handle, userImpl->m_handle, key, outValue);
	else
		return false;
}

Ref< Result > Lobby::join()
{
	if (!m_matchMakingProvider)
		return 0;

	Ref< Result > result = new Result();
	if (m_taskQueue->add(new TaskJoinLobby(
		m_matchMakingProvider,
		m_handle,
		result
	)))
		return result;
	else
		return 0;
}

bool Lobby::leave()
{
	if (!m_matchMakingProvider)
		return true;

	if (!m_matchMakingProvider->leaveLobby(m_handle))
		return false;

	m_matchMakingProvider = 0;
	return true;
}

bool Lobby::getParticipants(RefArray< IUser >& outUsers)
{
	if (!m_matchMakingProvider)
		return false;

	std::vector< uint64_t > userHandles;
	userHandles.reserve(64);

	m_matchMakingProvider->getLobbyParticipants(m_handle, userHandles);
	m_userCache->getMany(userHandles, (RefArray< User >&)outUsers);

	return true;
}

uint32_t Lobby::getParticipantCount() const
{
	uint32_t count;
	if (m_matchMakingProvider && m_matchMakingProvider->getLobbyParticipantCount(m_handle, count))
		return count;
	else
		return 0;
}

uint32_t Lobby::getMaxParticipantCount() const
{
	uint32_t count;
	if (m_matchMakingProvider && m_matchMakingProvider->getLobbyMaxParticipantCount(m_handle, count))
		return count;
	else
		return 0;
}

uint32_t Lobby::getFriendsCount() const
{
	uint32_t count;
	if (m_matchMakingProvider && m_matchMakingProvider->getLobbyFriendsCount(m_handle, count))
		return count;
	else
		return 0;
}

bool Lobby::invite(const IUser* user)
{
	const User* userImpl = dynamic_type_cast< const User* >(user);
	if (!userImpl)
		return false;

	if (m_matchMakingProvider && m_matchMakingProvider->inviteToLobby(m_handle, userImpl->m_handle))
		return true;
	else
		return true;
}

bool Lobby::setOwner(const IUser* user)
{
	if (!m_matchMakingProvider)
		return false;

	const User* userImpl = dynamic_type_cast< const User* >(user);
	if (!userImpl)
		return false;

	return m_matchMakingProvider->setLobbyOwner(m_handle, userImpl->m_handle);
}

const IUser* Lobby::getOwner() const
{
	if (!m_matchMakingProvider)
		return 0;

	uint64_t userHandle;
	if (!m_matchMakingProvider->getLobbyOwner(m_handle, userHandle))
		return 0;

	return m_userCache->get(userHandle);
}

Lobby::Lobby(IMatchMakingProvider* matchMakingProvider, UserCache* userCache, TaskQueue* taskQueue, uint64_t handle)
:	m_matchMakingProvider(matchMakingProvider)
,	m_userCache(userCache)
,	m_taskQueue(taskQueue)
,	m_handle(handle)
{
}

	}
}
