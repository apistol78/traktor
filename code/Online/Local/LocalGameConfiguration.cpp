#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberStl.h"
#include "Online/Local/LocalGameConfiguration.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.online.LocalGameConfiguration", 0, LocalGameConfiguration, IGameConfiguration)

void LocalGameConfiguration::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"dbName", m_dbName);
	s >> MemberStlList< std::wstring >(L"achievementIds", m_achievementIds);
	s >> MemberStlList< std::wstring >(L"leaderboardIds", m_leaderboardIds);
	s >> MemberStlList< std::wstring >(L"statsIds", m_statsIds);
}

	}
}
