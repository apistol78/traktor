#include "Core/Misc/TString.h"
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
			currentThread->sleep(100);
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
,	m_joinResult(false)
{
}

bool SteamMatchMaking::findMatchingLobbies(const LobbyFilter* filter, std::vector< uint64_t >& outLobbyHandles)
{
	m_outLobbies = &outLobbyHandles;

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

	if (filter->getCount() > 0)
		SteamMatchmaking()->AddRequestLobbyListResultCountFilter(filter->getCount());

	SteamAPICall_t hSteamAPICall = SteamMatchmaking()->RequestLobbyList();
	m_callbackLobbyMatch.Set(hSteamAPICall, this, &SteamMatchMaking::OnLobbyMatch);

	bool result = performCall(m_sessionManager, m_callbackLobbyMatch);

	if (!m_outLobbies)
		result = false;

	m_outLobbies = 0;
	return result;
}

bool SteamMatchMaking::createLobby(uint32_t maxUsers, uint64_t& outLobbyHandle)
{
	m_outLobby = &outLobbyHandle;

	SteamAPICall_t hSteamAPICall = SteamMatchmaking()->CreateLobby(k_ELobbyTypePublic, maxUsers);
	m_callbackLobbyCreated.Set(hSteamAPICall, this, &SteamMatchMaking::OnLobbyCreated);

	bool result = performCall(m_sessionManager, m_callbackLobbyCreated);

	if (!m_outLobby)
		result = false;

	m_outLobby = 0;
	return result;
}

bool SteamMatchMaking::joinLobby(uint64_t lobbyHandle)
{
	m_joinResult = false;

	CSteamID id(lobbyHandle);
	if (!id.IsValid())
		return false;

	SteamAPICall_t hSteamAPICall = SteamMatchmaking()->JoinLobby(id);
	m_callbackLobbyEnter.Set(hSteamAPICall, this, &SteamMatchMaking::OnLobbyEnter);

	bool result = performCall(m_sessionManager, m_callbackLobbyEnter);

	if (!m_joinResult)
		result = false;

	return result;
}

bool SteamMatchMaking::leaveLobby(uint64_t lobbyHandle)
{
	CSteamID id(lobbyHandle);
	if (!id.IsValid())
		return false;

	SteamMatchmaking()->LeaveLobby(lobbyHandle);
	return true;
}

bool SteamMatchMaking::setMetaValue(uint64_t lobbyHandle, const std::wstring& key, const std::wstring& value)
{
	CSteamID id(lobbyHandle);
	if (!id.IsValid())
		return false;

	return SteamMatchmaking()->SetLobbyData(
		id,
		wstombs(key).c_str(),
		wstombs(value).c_str()
	);
}

bool SteamMatchMaking::getMetaValue(uint64_t lobbyHandle, const std::wstring& key, std::wstring& outValue)
{
	CSteamID id(lobbyHandle);
	if (!id.IsValid())
		return false;

	const char* value = SteamMatchmaking()->GetLobbyData(id, wstombs(key).c_str());
	if (!value)
		return false;

	outValue = mbstows(value);
	return true;
}

bool SteamMatchMaking::setParticipantMetaValue(uint64_t lobbyHandle, const std::wstring& key, const std::wstring& value)
{
	CSteamID id(lobbyHandle);
	if (!id.IsValid())
		return false;

	SteamMatchmaking()->SetLobbyMemberData(
		id,
		wstombs(key).c_str(),
		wstombs(value).c_str()
	);

	return true;
}

bool SteamMatchMaking::getParticipantMetaValue(uint64_t lobbyHandle, uint64_t userHandle, const std::wstring& key, std::wstring& outValue)
{
	CSteamID lobbyId(lobbyHandle), userId(userHandle);
	if (!lobbyId.IsValid() || !userId.IsValid())
		return false;

	const char* value = SteamMatchmaking()->GetLobbyMemberData(lobbyId, userId, wstombs(key).c_str());
	if (!value)
		return false;

	outValue = mbstows(value);
	return true;
}

bool SteamMatchMaking::getParticipants(uint64_t lobbyHandle, std::vector< uint64_t >& outUserHandles)
{
	CSteamID id(lobbyHandle);
	if (!id.IsValid())
		return false;

	CSteamID myId = ::SteamUser()->GetSteamID();
	int32_t memberCount = SteamMatchmaking()->GetNumLobbyMembers(id);

	outUserHandles.reserve(memberCount);
	for (int32_t i = 0; i < memberCount; ++i)
	{
		CSteamID memberId = SteamMatchmaking()->GetLobbyMemberByIndex(lobbyHandle, i);
		if (memberId != myId)
			outUserHandles.push_back(memberId.ConvertToUint64());
	}

	return true;
}

bool SteamMatchMaking::getIndex(uint64_t lobbyHandle, int32_t& outIndex) const
{
	CSteamID id(lobbyHandle);
	if (!id.IsValid())
		return false;

	CSteamID myId = ::SteamUser()->GetSteamID();
	int32_t memberCount = SteamMatchmaking()->GetNumLobbyMembers(id);

	for (int32_t i = 0; i < memberCount; ++i)
	{
		CSteamID memberId = SteamMatchmaking()->GetLobbyMemberByIndex(lobbyHandle, i);
		if (memberId == myId)
		{
			outIndex = i;
			return true;
		}
	}

	return false;
}

void SteamMatchMaking::OnLobbyMatch(LobbyMatchList_t* pCallback, bool bIOFailure)
{
	T_ASSERT (m_outLobbies != 0);
	for (uint32_t i = 0; i < pCallback->m_nLobbiesMatching; ++i)
	{
		CSteamID lobbyId = SteamMatchmaking()->GetLobbyByIndex(i);
		if (!lobbyId.IsValid())
			continue;

		m_outLobbies->push_back(lobbyId.ConvertToUint64());
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

	}
}
