#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberStl.h"
#include "Online/Steam/SteamGameConfiguration.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.online.SteamGameConfiguration", 1, SteamGameConfiguration, IGameConfiguration)

SteamGameConfiguration::SteamGameConfiguration()
:	m_requestAttempts(10)
,	m_cloudEnabled(false)
,	m_allowP2PRelay(true)
{
}

bool SteamGameConfiguration::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"requestAttempts", m_requestAttempts);
	s >> Member< bool >(L"cloudEnabled", m_cloudEnabled);

	if (s.getVersion() >= 1)
		s >> Member< bool >(L"allowP2PRelay", m_allowP2PRelay);

	s >> MemberStlList< std::wstring >(L"achievementIds", m_achievementIds);
	s >> MemberStlList< std::wstring >(L"leaderboardIds", m_leaderboardIds);
	s >> MemberStlList< std::wstring >(L"statsIds", m_statsIds);
	return true;
}

	}
}
