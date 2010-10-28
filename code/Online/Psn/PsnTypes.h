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
};

	}
}

#endif	// traktor_online_PsnTypes_H
