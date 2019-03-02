#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Online/Gc/GcGameConfiguration.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.online.GcGameConfiguration", 2, GcGameConfiguration, IGameConfiguration)

void GcGameConfiguration::serialize(ISerializer& s)
{
	T_ASSERT (s.getVersion() >= 2);

	s >> MemberStlList< std::wstring >(L"achievementIds", m_achievementIds);
	s >> MemberStlList< std::wstring >(L"leaderboardIds", m_leaderboardIds);
	s >> MemberStlList< std::wstring >(L"statsIds", m_statsIds);
	s >> Member< std::wstring >(L"sharingClientId", m_sharingClientId);
	s >> Member< std::wstring >(L"sharingClientSecret", m_sharingClientSecret);
	s >> Member< std::wstring >(L"sharingRedirectURI", m_sharingRedirectURI);
}

	}
}
