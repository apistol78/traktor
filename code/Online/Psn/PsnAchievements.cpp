/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Log/Log.h"
#include "Online/Psn/PsnAchievements.h"
#include "Online/Psn/PsnLogError.h"

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

	int32_t err = sceNpTrophyRegisterContext(m_trophyContext, m_trophyHandle, &callbackTrophyStatus, NULL, 0);

	if (err != 0)
	{
		log::error << PsnLogError::getTrophyErrorString(err) << Endl;
		return false;
	}

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

int PsnAchievements::callbackTrophyStatus(SceNpTrophyContext context, SceNpTrophyStatus status, int completed, int total, void *arg)
{
	switch (status)
	{

	case SCE_NP_TROPHY_STATUS_NOT_INSTALLED:
		log::debug << L"Trophy configuration data is not installed." << Endl;
		break;
	case SCE_NP_TROPHY_STATUS_DATA_CORRUPT:
		log::debug << L"Trophy configuration data is corrupted." << Endl;
		break;
	case SCE_NP_TROPHY_STATUS_INSTALLED:
		log::debug << L"Trophy configuration data has been installed." << Endl;
		break;
	case SCE_NP_TROPHY_STATUS_REQUIRES_UPDATE:
		log::debug << L"A new trophy pack file is available in a patch." << Endl;
		break;
	case SCE_NP_TROPHY_STATUS_CHANGES_DETECTED:
		log::debug << L"Trophy configuration data was modified."  << Endl;
		break;
	case SCE_NP_TROPHY_STATUS_UNKNOWN:
		log::debug << L"Trophy unknown status, internal error" << Endl;
		break;
	case SCE_NP_TROPHY_STATUS_PROCESSING_SETUP:
		log::debug << L"Trophy processing setup. ";
		log::debug << completed << L"/" << total << Endl;
		break;
	case SCE_NP_TROPHY_STATUS_PROCESSING_PROGRESS:
		log::debug << L"Trophy processing. ";
		log::debug << completed << L"/" << total << Endl;
		break;
	case SCE_NP_TROPHY_STATUS_PROCESSING_FINALIZE:
		log::debug << L"Trophy processing termination processing. ";
		log::debug << completed << L"/" << total << Endl;
		break;
	case SCE_NP_TROPHY_STATUS_PROCESSING_COMPLETE:
		log::debug << L"Trophy processing completed." << Endl;
		break;
	default:
		break;
	}
	return 0;
}

	}
}
