#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Online/Gc/GcGameConfiguration.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.online.GcGameConfiguration", 1, GcGameConfiguration, IGameConfiguration)

void GcGameConfiguration::serialize(ISerializer& s)
{
	s >> MemberStlList< std::wstring >(L"achievementIds", m_achievementIds);
	s >> MemberStlList< std::wstring >(L"leaderboardIds", m_leaderboardIds);
	s >> MemberStlList< std::wstring >(L"statsIds", m_statsIds);

	if (s.getVersion() >= 1)
	{
		s >> Member< std::wstring >(L"kamcordDeveloperKey", m_kamcordDeveloperKey);
		s >> Member< std::wstring >(L"kamcordDeveloperSecret", m_kamcordDeveloperSecret);
		s >> Member< std::wstring >(L"kamcordAppName", m_kamcordAppName);
	}
}

	}
}
