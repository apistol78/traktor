#include "Core/Log/Log.h"
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

const uint32_t c_maxLobbyAge = 60*60;	// Seconds; if a lobby owner has been stuck for more than N seconds the lobby will not be accepted.

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
,	m_outLobby(0)
,	m_acceptedInvite(0)
,	m_joinedLobby(0)
,	m_joinResult(false)
,	m_callbackGameLobbyJoinRequested(this, &SteamMatchMaking::OnGameLobbyJoinRequested)
,	m_callbackLobbyDataUpdate(this, &SteamMatchMaking::OnLobbyDataUpdate)
{
	std::vector< std::wstring > argv;
	Split< std::wstring >::any(OS::getInstance().getCommandLine(), L" \t", argv);
	if (argv.size() >= 2)
	{
		for (size_t i = 0; i < argv.size() - 1; ++i)
		{
			if (argv[i] == L"+connect_lobby")
			{
				m_acceptedInvite = parseString< uint64_t >(argv[i + 1]);
				break;
			}
		}
	}
}

bool SteamMatchMaking::findMatchingLobbies(const LobbyFilter* filter, std::vector< uint64_t >& outLobbyHandles)
{
	leaveLobby(m_joinedLobby);

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

	switch (filter->getDistance())
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
	if (!result)
	{
		m_outLobbies.clear();
		return false;
	}

	uint32_t timeStamp = SteamUtils()->GetServerRealTime();

	// Wait for all lobby meta data to be received; max ten seconds.
	for (int32_t i = 0; i < 10000; i += 100)
	{
		if (!m_sessionManager->update())
			return false;

		bool uptodate = true;
		for (std::vector< FindLobby >::const_iterator i = m_outLobbies.begin(); i != m_outLobbies.end(); ++i)
		{
			if (!i->uptodate)
			{
				uptodate = false;
				break;
			}
		}
		if (uptodate)
		{
			log::debug << L"All found lobbies meta data up-to-date" << Endl;
			break;
		}

		ThreadManager::getInstance().getCurrentThread()->sleep(100);
	}

	// Check time stamp; only keep lobbies which isn't "stale".
	for (std::vector< FindLobby >::const_iterator i = m_outLobbies.begin(); i != m_outLobbies.end(); ++i)
	{
		if (i->succeeded)
		{
			const char* value = SteamMatchmaking()->GetLobbyData(i->lobbyId, "__PRIVATE_TIME_STAMP__");
			uint32_t lobbyTimeStamp = value ? parseString< uint32_t >(value) : 0;

			if (lobbyTimeStamp == 0 || lobbyTimeStamp > timeStamp || timeStamp - lobbyTimeStamp < c_maxLobbyAge)
				outLobbyHandles.push_back(i->lobbyId.ConvertToUint64());
			else
				log::warning << L"Lobby ignored; too old timestamp (" << (timeStamp - lobbyTimeStamp) << L" seconds, max " << c_maxLobbyAge << L")" << Endl;
		}
		else
			log::warning << L"Lobby ignored; unable to refresh meta data" << Endl;
	}

	m_outLobbies.clear();
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

	m_outLobby = &outLobbyHandle;

	SteamAPICall_t hSteamAPICall = SteamMatchmaking()->CreateLobby(lobbyAccess[access], maxUsers);
	m_callbackLobbyCreated.Set(hSteamAPICall, this, &SteamMatchMaking::OnLobbyCreated);

	bool result = performCall(m_sessionManager, m_callbackLobbyCreated);

	if (!m_outLobby)
		result = false;

	m_outLobby = 0;

	if (result)
	{
		m_joinedLobby = outLobbyHandle;

		uint32_t timeStamp = SteamUtils()->GetServerRealTime();
		SteamMatchmaking()->SetLobbyData(
			uint64(m_joinedLobby),
			"__PRIVATE_TIME_STAMP__",
			wstombs(toString(timeStamp)).c_str()
		);
	}

	return result;
}

bool SteamMatchMaking::acceptLobby(uint64_t& outLobbyHandle)
{
	if (m_acceptedInvite == 0)
		return false;

	leaveLobby(m_joinedLobby);

	outLobbyHandle = m_acceptedInvite;
	m_acceptedInvite = 0;

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
		m_joinedLobby = lobbyHandle;

	return result;
}

bool SteamMatchMaking::leaveLobby(uint64_t lobbyHandle)
{
	if (m_joinedLobby != lobbyHandle)
		return false;

	SteamMatchmaking()->LeaveLobby(uint64(lobbyHandle));
	m_joinedLobby = 0;
	return true;
}

bool SteamMatchMaking::setMetaValue(uint64_t lobbyHandle, const std::wstring& key, const std::wstring& value)
{
	T_ASSERT_M (m_joinedLobby != 0, L"Not in any lobby");
	T_ASSERT_M (m_joinedLobby == lobbyHandle, L"Incorrect lobby");
	return SteamMatchmaking()->SetLobbyData(
		uint64(lobbyHandle),
		wstombs(key).c_str(),
		wstombs(value).c_str()
	);
}

bool SteamMatchMaking::getMetaValue(uint64_t lobbyHandle, const std::wstring& key, std::wstring& outValue)
{
	const char* value = SteamMatchmaking()->GetLobbyData(uint64(lobbyHandle), wstombs(key).c_str());
	if (!value)
		return false;

	outValue = mbstows(value);
	return true;
}

bool SteamMatchMaking::setParticipantMetaValue(uint64_t lobbyHandle, const std::wstring& key, const std::wstring& value)
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

bool SteamMatchMaking::getParticipantMetaValue(uint64_t lobbyHandle, uint64_t userHandle, const std::wstring& key, std::wstring& outValue)
{
	T_ASSERT_M (m_joinedLobby != 0, L"Not in any lobby");
	T_ASSERT_M (m_joinedLobby == lobbyHandle, L"Incorrect lobby");

	const char* value = SteamMatchmaking()->GetLobbyMemberData(uint64(lobbyHandle), uint64(userHandle), wstombs(key).c_str());
	if (!value)
		return false;

	outValue = mbstows(value);
	return true;
}

bool SteamMatchMaking::getParticipants(uint64_t lobbyHandle, std::vector< uint64_t >& outUserHandles)
{
	T_ASSERT_M (m_joinedLobby != 0, L"Not in any lobby");
	T_ASSERT_M (m_joinedLobby == lobbyHandle, L"Incorrect lobby");

	CSteamID myId = ::SteamUser()->GetSteamID();
	int32_t memberCount = SteamMatchmaking()->GetNumLobbyMembers(uint64(lobbyHandle));

	outUserHandles.reserve(memberCount);
	for (int32_t i = 0; i < memberCount; ++i)
	{
		CSteamID memberId = SteamMatchmaking()->GetLobbyMemberByIndex(uint64(lobbyHandle), i);
		if (memberId != myId)
			outUserHandles.push_back(memberId.ConvertToUint64());
	}

	return true;
}

bool SteamMatchMaking::getParticipantCount(uint64_t lobbyHandle, uint32_t& outCount) const
{
	outCount = SteamMatchmaking()->GetNumLobbyMembers(uint64(lobbyHandle));
	return true;
}

bool SteamMatchMaking::getMaxParticipantCount(uint64_t lobbyHandle, uint32_t& outCount) const
{
	outCount = SteamMatchmaking()->GetLobbyMemberLimit(uint64(lobbyHandle));
	return true;
}

bool SteamMatchMaking::getFriendsCount(uint64_t lobbyHandle, uint32_t& outCount) const
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

bool SteamMatchMaking::invite(uint64_t lobbyHandle, uint64_t userHandle)
{
	T_ASSERT_M (m_joinedLobby != 0, L"Not in any lobby");
	T_ASSERT_M (m_joinedLobby == lobbyHandle, L"Incorrect lobby");
	return SteamMatchmaking()->InviteUserToLobby(uint64(lobbyHandle), uint64(userHandle));
}

bool SteamMatchMaking::getIndex(uint64_t lobbyHandle, int32_t& outIndex) const
{
	T_ASSERT_M (m_joinedLobby != 0, L"Not in any lobby");
	T_ASSERT_M (m_joinedLobby == lobbyHandle, L"Incorrect lobby");

	CSteamID myId = ::SteamUser()->GetSteamID();
	int32_t memberCount = SteamMatchmaking()->GetNumLobbyMembers(uint64(lobbyHandle));

	for (int32_t i = 0; i < memberCount; ++i)
	{
		CSteamID memberId = SteamMatchmaking()->GetLobbyMemberByIndex(uint64(lobbyHandle), i);
		if (memberId == myId)
		{
			outIndex = i;
			return true;
		}
	}

	return false;
}

bool SteamMatchMaking::setOwner(uint64_t lobbyHandle, uint64_t userHandle) const
{
	T_ASSERT_M (m_joinedLobby != 0, L"Not in any lobby");
	T_ASSERT_M (m_joinedLobby == lobbyHandle, L"Incorrect lobby");
	return SteamMatchmaking()->SetLobbyOwner(uint64(lobbyHandle), uint64(userHandle));
}

bool SteamMatchMaking::getOwner(uint64_t lobbyHandle, uint64_t& outUserHandle) const
{
	T_ASSERT_M (m_joinedLobby != 0, L"Not in any lobby");
	T_ASSERT_M (m_joinedLobby == lobbyHandle, L"Incorrect lobby");
	outUserHandle = SteamMatchmaking()->GetLobbyOwner(uint64(lobbyHandle)).ConvertToUint64();
	return true;
}

void SteamMatchMaking::update()
{
	if (m_joinedLobby == 0)
		return;

	// In case I'm the lobby owner then update lobby timestamp.
	if (SteamMatchmaking()->GetLobbyOwner(uint64(m_joinedLobby)) == ::SteamUser()->GetSteamID())
	{
		uint32_t timeStamp = SteamUtils()->GetServerRealTime();
		SteamMatchmaking()->SetLobbyData(
			uint64(m_joinedLobby),
			"__PRIVATE_TIME_STAMP__",
			wstombs(toString(timeStamp)).c_str()
		);
	}
}

void SteamMatchMaking::OnLobbyMatch(LobbyMatchList_t* pCallback, bool bIOFailure)
{
	for (uint32_t i = 0; i < pCallback->m_nLobbiesMatching; ++i)
	{
		CSteamID lobbyId = SteamMatchmaking()->GetLobbyByIndex(i);
		if (!lobbyId.IsValid())
			continue;

		if (!SteamMatchmaking()->RequestLobbyData(lobbyId))
			continue;

		FindLobby fl;
		fl.lobbyId = lobbyId;
		fl.uptodate = false;
		m_outLobbies.push_back(fl);
	}
}

void SteamMatchMaking::OnLobbyCreated(LobbyCreated_t* pCallback, bool bIOFailure)
{
	T_ASSERT (m_outLobby != 0);
	if (pCallback->m_eResult == k_EResultOK)
		*m_outLobby = pCallback->m_ulSteamIDLobby;
	else
		m_outLobby = 0;
}

void SteamMatchMaking::OnLobbyEnter(LobbyEnter_t* pCallback, bool bIOFailure)
{
	m_joinResult = bool(pCallback->m_EChatRoomEnterResponse == k_EChatRoomEnterResponseSuccess);
}

void SteamMatchMaking::OnGameLobbyJoinRequested(GameLobbyJoinRequested_t* pCallback)
{
	m_acceptedInvite = pCallback->m_steamIDLobby.ConvertToUint64();
}

void SteamMatchMaking::OnLobbyDataUpdate(LobbyDataUpdate_t* pCallback)
{
    for (std::vector< FindLobby >::iterator i = m_outLobbies.begin(); i != m_outLobbies.end(); ++i)
    {
		if (i->lobbyId == pCallback->m_ulSteamIDLobby)
		{
			i->uptodate = true;
			i->succeeded = bool(pCallback->m_bSuccess != 0);
			break;
		}
    }
}

	}
}
