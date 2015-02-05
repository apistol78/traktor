#include "Core/Misc/CommandLine.h"
#include "Core/Misc/TString.h"
#include "Core/System/OS.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Online/LobbyFilter.h"
#include "Online/Steam/SteamMatchMaking.h"
#include "Online/Steam/SteamSessionManager.h"

namespace traktor
{
	namespace online
	{
		namespace
		{

template < typename CallType >
bool performCall(SteamSessionManager* sessionManager, CallType& call)
{
	Thread* currentThread = ThreadManager::getInstance().getCurrentThread();
	while (call.IsActive())
	{
		if (!sessionManager->update())
			return false;
		if (!call.IsActive())
			break;
		if (currentThread)
		{
			if (!currentThread->stopped())
				currentThread->sleep(100);
			else
				return false;
		}
	}
	return true;
}

ELobbyComparison translateComparison(LobbyFilter::ComparisonType comparison)
{
	switch (comparison)
	{
	default:
	case LobbyFilter::CtEqual:
		return k_ELobbyComparisonEqual;
	case LobbyFilter::CtNotEqual:
		return k_ELobbyComparisonNotEqual;
	case LobbyFilter::CtLess:
		return k_ELobbyComparisonLessThan;
	case LobbyFilter::CtLessEqual:
		return k_ELobbyComparisonEqualToOrLessThan;
	case LobbyFilter::CtGreater:
		return k_ELobbyComparisonGreaterThan;
	case LobbyFilter::CtGreaterEqual:
		return k_ELobbyComparisonEqualToOrGreaterThan;
	}
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.SteamMatchMaking", SteamMatchMaking, IMatchMakingProvider)

SteamMatchMaking::SteamMatchMaking(SteamSessionManager* sessionManager)
:	m_sessionManager(sessionManager)
,	m_outLobbies(0)
,	m_outLobbyOrParty(0)
,	m_acceptedLobbyInvite(0)
,	m_acceptedPartyInvite(0)
,	m_joinedLobby(0)
,	m_joinResult(false)
,	m_callbackGameLobbyJoinRequested(this, &SteamMatchMaking::OnGameLobbyJoinRequested)
,	m_callbackChatDataUpdate(this, &SteamMatchMaking::OnLobbyChatUpdate)
{
	std::vector< std::wstring > argv;
	Split< std::wstring >::any(OS::getInstance().getCommandLine(), L" \t", argv);
	if (argv.size() >= 2)
	{
		for (size_t i = 0; i < argv.size() - 1; ++i)
		{
			if (argv[i] == L"+connect_lobby")
			{
				uint64_t lobbyHandle = parseString< uint64_t >(argv[i + 1]);
				const char* value = SteamMatchmaking()->GetLobbyData(uint64(lobbyHandle), "__LOBBY_TYPE__");
				if (value)
				{
					if (strcmp(value, "LOBBY"))
						m_acceptedLobbyInvite = lobbyHandle;
					else if (strcmp(value, "PARTY"))
						m_acceptedPartyInvite = lobbyHandle;
				}
				break;
			}
		}
	}
}

bool SteamMatchMaking::findMatchingLobbies(const LobbyFilter* filter, std::vector< uint64_t >& outLobbyHandles)
{
	leaveLobby(m_joinedLobby);

	m_outLobbies = &outLobbyHandles;

	for (int32_t distance = LobbyFilter::DtUnspecified; distance <= filter->getDistance(); ++distance)
	{
		const std::vector< LobbyFilter::StringComparison >& stringComparisons = filter->getStringComparisons();
		for (std::vector< LobbyFilter::StringComparison >::const_iterator i = stringComparisons.begin(); i != stringComparisons.end(); ++i)
		{
			SteamMatchmaking()->AddRequestLobbyListStringFilter(
				wstombs(i->key).c_str(),
				wstombs(i->value).c_str(),
				translateComparison(i->comparison)
			);
		}

		const std::vector< LobbyFilter::NumberComparison >& numberComparisons = filter->getNumberComparisons();
		for (std::vector< LobbyFilter::NumberComparison >::const_iterator i = numberComparisons.begin(); i != numberComparisons.end(); ++i)
		{
			SteamMatchmaking()->AddRequestLobbyListNumericalFilter(
				wstombs(i->key).c_str(),
				i->value,
				translateComparison(i->comparison)
			);
		}

		switch (distance)
		{
		case LobbyFilter::DtLocal:
			SteamMatchmaking()->AddRequestLobbyListDistanceFilter(k_ELobbyDistanceFilterClose);
			break;
		case LobbyFilter::DtNear:
			SteamMatchmaking()->AddRequestLobbyListDistanceFilter(k_ELobbyDistanceFilterDefault);
			break;
		case LobbyFilter::DtFar:
			SteamMatchmaking()->AddRequestLobbyListDistanceFilter(k_ELobbyDistanceFilterFar);
			break;
		case LobbyFilter::DtInfinity:
			SteamMatchmaking()->AddRequestLobbyListDistanceFilter(k_ELobbyDistanceFilterWorldwide);
			break;
		default:
			break;
		}

		if (filter->getSlots() > 0)
			SteamMatchmaking()->AddRequestLobbyListFilterSlotsAvailable(filter->getSlots());

		if (filter->getCount() > 0)
			SteamMatchmaking()->AddRequestLobbyListResultCountFilter(filter->getCount());

		SteamAPICall_t hSteamAPICall = SteamMatchmaking()->RequestLobbyList();
		m_callbackLobbyMatch.Set(hSteamAPICall, this, &SteamMatchMaking::OnLobbyMatch);

		bool result = performCall(m_sessionManager, m_callbackLobbyMatch);

		if (!result || !m_outLobbies)
			return false;
	}

	m_outLobbies = 0;
	return true;
}

bool SteamMatchMaking::createLobby(uint32_t maxUsers, LobbyAccess access, uint64_t& outLobbyHandle)
{
	const ELobbyType lobbyAccess[] =
	{
		k_ELobbyTypePublic,
		k_ELobbyTypePrivate,
		k_ELobbyTypeFriendsOnly
	};

	leaveLobby(m_joinedLobby);

	m_outLobbyOrParty = &outLobbyHandle;

	SteamAPICall_t hSteamAPICall = SteamMatchmaking()->CreateLobby(lobbyAccess[access], maxUsers);
	m_callbackLobbyCreated.Set(hSteamAPICall, this, &SteamMatchMaking::OnLobbyCreated);

	bool result = performCall(m_sessionManager, m_callbackLobbyCreated);

	if (!m_outLobbyOrParty)
		result = false;

	m_outLobbyOrParty = 0;

	if (result)
	{
		m_joinedLobby = outLobbyHandle;
		SteamMatchmaking()->SetLobbyData(uint64(outLobbyHandle), "__LOBBY_TYPE__", "LOBBY");
		updateLobbyParticipants();
	}

	return result;
}

bool SteamMatchMaking::acceptLobby(uint64_t& outLobbyHandle)
{
	if (m_acceptedLobbyInvite == 0)
		return false;

	leaveLobby(m_joinedLobby);

	outLobbyHandle = m_acceptedLobbyInvite;
	m_acceptedLobbyInvite = 0;

	return true;
}

bool SteamMatchMaking::joinLobby(uint64_t lobbyHandle)
{
	leaveLobby(m_joinedLobby);

	m_joinResult = false;

	SteamAPICall_t hSteamAPICall = SteamMatchmaking()->JoinLobby(uint64(lobbyHandle));
	m_callbackLobbyEnter.Set(hSteamAPICall, this, &SteamMatchMaking::OnLobbyEnter);

	bool result = performCall(m_sessionManager, m_callbackLobbyEnter);

	if (!m_joinResult)
		result = false;

	if (result)
	{
		m_joinedLobby = lobbyHandle;
		updateLobbyParticipants();
	}

	return result;
}

bool SteamMatchMaking::leaveLobby(uint64_t lobbyHandle)
{
	if (m_joinedLobby != lobbyHandle)
		return false;

	SteamMatchmaking()->LeaveLobby(uint64(lobbyHandle));
	m_joinedLobby = 0;
	m_lobbyParticipants.resize(0);
	return true;
}

bool SteamMatchMaking::setLobbyMetaValue(uint64_t lobbyHandle, const std::wstring& key, const std::wstring& value)
{
	T_ASSERT_M (m_joinedLobby != 0, L"Not in any lobby");
	T_ASSERT_M (m_joinedLobby == lobbyHandle, L"Incorrect lobby");
	return SteamMatchmaking()->SetLobbyData(
		uint64(lobbyHandle),
		wstombs(key).c_str(),
		wstombs(value).c_str()
	);
}

bool SteamMatchMaking::getLobbyMetaValue(uint64_t lobbyHandle, const std::wstring& key, std::wstring& outValue)
{
	const char* value = SteamMatchmaking()->GetLobbyData(uint64(lobbyHandle), wstombs(key).c_str());
	if (!value)
		return false;

	outValue = mbstows(value);
	return true;
}

bool SteamMatchMaking::setLobbyParticipantMetaValue(uint64_t lobbyHandle, const std::wstring& key, const std::wstring& value)
{
	T_ASSERT_M (m_joinedLobby != 0, L"Not in any lobby");
	T_ASSERT_M (m_joinedLobby == lobbyHandle, L"Incorrect lobby");
	SteamMatchmaking()->SetLobbyMemberData(
		uint64(lobbyHandle),
		wstombs(key).c_str(),
		wstombs(value).c_str()
	);
	return true;
}

bool SteamMatchMaking::getLobbyParticipantMetaValue(uint64_t lobbyHandle, uint64_t userHandle, const std::wstring& key, std::wstring& outValue)
{
	T_ASSERT_M (m_joinedLobby != 0, L"Not in any lobby");
	T_ASSERT_M (m_joinedLobby == lobbyHandle, L"Incorrect lobby");

	const char* value = SteamMatchmaking()->GetLobbyMemberData(uint64(lobbyHandle), uint64(userHandle), wstombs(key).c_str());
	if (!value)
		return false;

	outValue = mbstows(value);
	return true;
}

bool SteamMatchMaking::getLobbyParticipants(uint64_t lobbyHandle, std::vector< uint64_t >& outUserHandles)
{
	T_ASSERT_M (m_joinedLobby != 0, L"Not in any lobby");
	T_ASSERT_M (m_joinedLobby == lobbyHandle, L"Incorrect lobby");

	outUserHandles = m_lobbyParticipants;
	return true;
}

bool SteamMatchMaking::getLobbyParticipantCount(uint64_t lobbyHandle, uint32_t& outCount) const
{
	outCount = SteamMatchmaking()->GetNumLobbyMembers(uint64(lobbyHandle));
	return true;
}

bool SteamMatchMaking::getLobbyMaxParticipantCount(uint64_t lobbyHandle, uint32_t& outCount) const
{
	outCount = SteamMatchmaking()->GetLobbyMemberLimit(uint64(lobbyHandle));
	return true;
}

bool SteamMatchMaking::getLobbyFriendsCount(uint64_t lobbyHandle, uint32_t& outCount) const
{
	outCount = 0;

	int32_t friendsCount = SteamFriends()->GetFriendCount(k_EFriendFlagImmediate);
	for (int32_t i = 0; i < friendsCount; ++i) 
	{
		FriendGameInfo_t friendGameInfo;
		CSteamID steamIDFriend = SteamFriends()->GetFriendByIndex(i, k_EFriendFlagImmediate);
		if (SteamFriends()->GetFriendGamePlayed(steamIDFriend, &friendGameInfo) && friendGameInfo.m_steamIDLobby.IsValid())
		{
			if (friendGameInfo.m_steamIDLobby.ConvertToUint64() == lobbyHandle)
				++outCount;
		}
	}

	return true;
}

bool SteamMatchMaking::inviteToLobby(uint64_t lobbyHandle, uint64_t userHandle)
{
	T_ASSERT_M (m_joinedLobby != 0, L"Not in any lobby");
	T_ASSERT_M (m_joinedLobby == lobbyHandle, L"Incorrect lobby");
	return SteamMatchmaking()->InviteUserToLobby(uint64(lobbyHandle), uint64(userHandle));
}

bool SteamMatchMaking::setLobbyOwner(uint64_t lobbyHandle, uint64_t userHandle) const
{
	T_ASSERT_M (m_joinedLobby != 0, L"Not in any lobby");
	T_ASSERT_M (m_joinedLobby == lobbyHandle, L"Incorrect lobby");
	return SteamMatchmaking()->SetLobbyOwner(uint64(lobbyHandle), uint64(userHandle));
}

bool SteamMatchMaking::getLobbyOwner(uint64_t lobbyHandle, uint64_t& outUserHandle) const
{
	T_ASSERT_M (m_joinedLobby != 0, L"Not in any lobby");
	T_ASSERT_M (m_joinedLobby == lobbyHandle, L"Incorrect lobby");
	outUserHandle = SteamMatchmaking()->GetLobbyOwner(uint64(lobbyHandle)).ConvertToUint64();
	return true;
}

bool SteamMatchMaking::createParty(uint64_t& outPartyHandle)
{
	leaveParty(m_joinedParty);

	m_outLobbyOrParty = &outPartyHandle;

	SteamAPICall_t hSteamAPICall = SteamMatchmaking()->CreateLobby(k_ELobbyTypeInvisible, 16);
	m_callbackLobbyCreated.Set(hSteamAPICall, this, &SteamMatchMaking::OnLobbyCreated);

	bool result = performCall(m_sessionManager, m_callbackLobbyCreated);

	if (!m_outLobbyOrParty)
		result = false;

	m_outLobbyOrParty = 0;

	if (result)
	{
		m_joinedParty = outPartyHandle;
		SteamMatchmaking()->SetLobbyData(uint64(outPartyHandle), "__LOBBY_TYPE__", "PARTY");
		updatePartyParticipants();
	}

	return result;
}

bool SteamMatchMaking::acceptParty(uint64_t& outPartyHandle)
{
	if (m_acceptedPartyInvite == 0)
		return false;

	leaveParty(m_joinedParty);

	outPartyHandle = m_acceptedPartyInvite;
	m_acceptedPartyInvite = 0;

	return true;
}

bool SteamMatchMaking::joinParty(uint64_t partyHandle)
{
	leaveParty(m_joinedParty);

	m_joinResult = false;

	SteamAPICall_t hSteamAPICall = SteamMatchmaking()->JoinLobby(uint64(partyHandle));
	m_callbackLobbyEnter.Set(hSteamAPICall, this, &SteamMatchMaking::OnLobbyEnter);

	bool result = performCall(m_sessionManager, m_callbackLobbyEnter);

	if (!m_joinResult)
		result = false;

	if (result)
	{
		m_joinedParty = partyHandle;
		updatePartyParticipants();
	}

	return result;
}

bool SteamMatchMaking::leaveParty(uint64_t partyHandle)
{
	if (m_joinedParty != partyHandle)
		return false;

	SteamMatchmaking()->LeaveLobby(uint64(partyHandle));
	m_joinedParty = 0;
	m_partyParticipants.resize(0);
	return true;
}

bool SteamMatchMaking::setPartyMetaValue(uint64_t partyHandle, const std::wstring& key, const std::wstring& value)
{
	T_ASSERT_M (m_joinedParty != 0, L"Not in any party");
	T_ASSERT_M (m_joinedParty == partyHandle, L"Incorrect party");
	return SteamMatchmaking()->SetLobbyData(
		uint64(partyHandle),
		wstombs(key).c_str(),
		wstombs(value).c_str()
	);
}

bool SteamMatchMaking::getPartyMetaValue(uint64_t partyHandle, const std::wstring& key, std::wstring& outValue)
{
	const char* value = SteamMatchmaking()->GetLobbyData(uint64(partyHandle), wstombs(key).c_str());
	if (!value)
		return false;

	outValue = mbstows(value);
	return true;
}

bool SteamMatchMaking::setPartyParticipantMetaValue(uint64_t partyHandle, const std::wstring& key, const std::wstring& value)
{
	T_ASSERT_M (m_joinedParty != 0, L"Not in any party");
	T_ASSERT_M (m_joinedParty == partyHandle, L"Incorrect party");
	SteamMatchmaking()->SetLobbyMemberData(
		uint64(partyHandle),
		wstombs(key).c_str(),
		wstombs(value).c_str()
	);
	return true;
}

bool SteamMatchMaking::getPartyParticipantMetaValue(uint64_t partyHandle, uint64_t userHandle, const std::wstring& key, std::wstring& outValue)
{
	T_ASSERT_M (m_joinedParty != 0, L"Not in any party");
	T_ASSERT_M (m_joinedParty == partyHandle, L"Incorrect party");

	const char* value = SteamMatchmaking()->GetLobbyMemberData(uint64(partyHandle), uint64(userHandle), wstombs(key).c_str());
	if (!value)
		return false;

	outValue = mbstows(value);
	return true;
}

bool SteamMatchMaking::getPartyParticipants(uint64_t partyHandle, std::vector< uint64_t >& outUserHandles)
{
	T_ASSERT_M (m_joinedParty != 0, L"Not in any party");
	T_ASSERT_M (m_joinedParty == partyHandle, L"Incorrect party");

	outUserHandles = m_partyParticipants;
	return true;
}

bool SteamMatchMaking::getPartyParticipantCount(uint64_t partyHandle, uint32_t& outCount) const
{
	outCount = SteamMatchmaking()->GetNumLobbyMembers(uint64(partyHandle));
	return true;
}

bool SteamMatchMaking::inviteToParty(uint64_t partyHandle, uint64_t userHandle)
{
	T_ASSERT_M (m_joinedParty != 0, L"Not in any party");
	T_ASSERT_M (m_joinedParty == partyHandle, L"Incorrect party");
	return SteamMatchmaking()->InviteUserToLobby(uint64(partyHandle), uint64(userHandle));
}

void SteamMatchMaking::updateLobbyParticipants()
{
	T_ASSERT_M (m_joinedLobby != 0, L"Not in any lobby");

	CSteamID myId = ::SteamUser()->GetSteamID();
	int32_t memberCount = SteamMatchmaking()->GetNumLobbyMembers(uint64(m_joinedLobby));

	m_lobbyParticipants.resize(0);
	m_lobbyParticipants.reserve(memberCount);
	for (int32_t i = 0; i < memberCount; ++i)
	{
		CSteamID memberId = SteamMatchmaking()->GetLobbyMemberByIndex(uint64(m_joinedLobby), i);
		if (memberId != myId)
			m_lobbyParticipants.push_back(memberId.ConvertToUint64());
	}
}

void SteamMatchMaking::updatePartyParticipants()
{
	T_ASSERT_M (m_joinedParty != 0, L"Not in any party");

	CSteamID myId = ::SteamUser()->GetSteamID();
	int32_t memberCount = SteamMatchmaking()->GetNumLobbyMembers(uint64(m_joinedParty));

	m_partyParticipants.resize(0);
	m_partyParticipants.reserve(memberCount);
	for (int32_t i = 0; i < memberCount; ++i)
	{
		CSteamID memberId = SteamMatchmaking()->GetLobbyMemberByIndex(uint64(m_joinedParty), i);
		if (memberId != myId)
			m_partyParticipants.push_back(memberId.ConvertToUint64());
	}
}

void SteamMatchMaking::OnLobbyMatch(LobbyMatchList_t* pCallback, bool bIOFailure)
{
	T_ASSERT (m_outLobbies != 0);
	for (uint32_t i = 0; i < pCallback->m_nLobbiesMatching; ++i)
	{
		CSteamID lobbyId = SteamMatchmaking()->GetLobbyByIndex(i);
		if (!lobbyId.IsValid())
			continue;

		uint64_t id = lobbyId.ConvertToUint64();
		if (std::find(m_outLobbies->begin(), m_outLobbies->end(), id) == m_outLobbies->end())
			m_outLobbies->push_back(id);
	}
}

void SteamMatchMaking::OnLobbyCreated(LobbyCreated_t* pCallback, bool bIOFailure)
{
	T_ASSERT (m_outLobbyOrParty != 0);
	if (pCallback->m_eResult == k_EResultOK)
		*m_outLobbyOrParty = pCallback->m_ulSteamIDLobby;
	else
		m_outLobbyOrParty = 0;
}

void SteamMatchMaking::OnLobbyEnter(LobbyEnter_t* pCallback, bool bIOFailure)
{
	m_joinResult = bool(pCallback->m_EChatRoomEnterResponse == k_EChatRoomEnterResponseSuccess);
}

void SteamMatchMaking::OnGameLobbyJoinRequested(GameLobbyJoinRequested_t* pCallback)
{
	const char* value = SteamMatchmaking()->GetLobbyData(pCallback->m_steamIDLobby, "__LOBBY_TYPE__");
	if (value)
	{
		if (strcmp(value, "LOBBY"))
			m_acceptedLobbyInvite = pCallback->m_steamIDLobby.ConvertToUint64();
		else if (strcmp(value, "PARTY"))
			m_acceptedPartyInvite = pCallback->m_steamIDLobby.ConvertToUint64();
	}
}

void SteamMatchMaking::OnLobbyChatUpdate(LobbyChatUpdate_t* pCallback)
{
	if (pCallback->m_ulSteamIDLobby == m_joinedLobby)
		updateLobbyParticipants();

	if (pCallback->m_ulSteamIDLobby == m_joinedParty)
		updatePartyParticipants();
}

	}
}
