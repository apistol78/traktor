/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_online_PsnTypes_H
#define traktor_online_PsnTypes_H

#include <np.h>

namespace traktor
{
	namespace online
	{

struct PsnAchievementDesc
{
	const wchar_t* id;
	SceNpTrophyId trophyId;
};

struct PsnCreateDesc
{
	const PsnAchievementDesc* achievements;
	const wchar_t** leaderboardIds;
	const wchar_t** statIds;
	const SceNpCommunicationId* communicationId;
	const SceNpCommunicationPassphrase* passphrase;
	const SceNpCommunicationSignature* signature;
	const uint8_t* saveIconBuffer;
	int32_t saveIconSize;
};

	}
}

#endif	// traktor_online_PsnTypes_H
