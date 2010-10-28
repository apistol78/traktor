#include "Online/Psn/PsnAchievements.h"

namespace traktor
{
	namespace online
	{
		namespace
		{

SceNpTrophyId lookupTrophyId(const PsnAchievementDesc* achievements, const std::wstring& id)
{
	for (const PsnAchievementDesc* achievement = achievements; achievement->id; ++achievement)
	{
		if (achievement->id == id)
			return achievement->trophyId;
	}
	return SCE_NP_TROPHY_INVALID_TROPHY_ID;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.PsnAchievements", PsnAchievements, IAchievementsProvider)

PsnAchievements::PsnAchievements(const PsnAchievementDesc* achievements, SceNpTrophyContext trophyContext, SceNpTrophyHandle trophyHandle)
:	m_achievements(achievements)
,	m_trophyContext(trophyContext)
,	m_trophyHandle(trophyHandle)
{
}

bool PsnAchievements::enumerate(std::map< std::wstring, bool >& outAchievements)
{
	SceNpTrophyDetails details;
	SceNpTrophyData data;

	for (const PsnAchievementDesc* achievement = m_achievements; achievement->id; ++achievement)
	{
		std::memset(&details, 0, sizeof(details));
		std::memset(&data, 0, sizeof(data));

		int32_t err = sceNpTrophyGetTrophyInfo(
			m_trophyContext,
			m_trophyHandle,
			achievement->trophyId,
			&details,
			&data
		);
		if (err != CELL_OK)
			return false;

		outAchievements.insert(std::make_pair(
			achievement->id,
			data.unlocked
		));
	}

	return true;
}

bool PsnAchievements::set(const std::wstring& achievementId, bool reward)
{
	if (!reward)
		return false;

	SceNpTrophyId trophyId = lookupTrophyId(m_achievements, achievementId);
	if (trophyId == SCE_NP_TROPHY_INVALID_TROPHY_ID)
		return false;

	SceNpTrophyId platinumId = SCE_NP_TROPHY_INVALID_TROPHY_ID;
	int32_t err = sceNpTrophyUnlockTrophy(m_trophyContext, m_trophyHandle, trophyId, &platinumId);
	if (err != CELL_OK)
		return false;

	return true;
}

	}
}
