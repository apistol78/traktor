#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Online/Editor/AchievementDesc.h"
#include "Online/Editor/AchievementsAsset.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.online.AchievementsAsset", 0, AchievementsAsset, ISerializable)

void AchievementsAsset::add(const AchievementDesc* achievement)
{
	m_achievements.push_back(achievement);
}

const RefArray< const AchievementDesc >& AchievementsAsset::get() const
{
	return m_achievements;
}

bool AchievementsAsset::serialize(ISerializer& s)
{
	return s >> MemberRefArray< const AchievementDesc >(L"achievements", m_achievements);
}

	}
}
