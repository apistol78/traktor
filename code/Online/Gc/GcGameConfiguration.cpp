#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Online/Gc/GcGameConfiguration.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.online.GcGameConfiguration", 0, GcGameConfiguration, IGameConfiguration)

void GcGameConfiguration::serialize(ISerializer& s)
{
	s >> MemberStlList< std::wstring >(L"achievementIds", m_achievementIds);
	s >> MemberStlList< std::wstring >(L"leaderboardIds", m_leaderboardIds);
	s >> MemberStlList< std::wstring >(L"statsIds", m_statsIds);
}

	}
}
