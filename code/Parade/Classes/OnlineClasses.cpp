#include "Core/Misc/String.h"
#include "Core/Serialization/ISerializable.h"
#include "Online/IAchievements.h"
#include "Online/ILeaderboards.h"
#include "Online/ILobby.h"
#include "Online/IMatchMaking.h"
#include "Online/ISaveData.h"
#include "Online/ISessionManager.h"
#include "Online/IStatistics.h"
#include "Online/IUser.h"
#include "Online/LobbyFilter.h"
#include "Parade/Classes/OnlineClasses.h"
#include "Script/AutoScriptClass.h"
#include "Script/Boxes.h"
#include "Script/IScriptManager.h"

namespace traktor
{
	namespace parade
	{
		namespace
		{

bool translateComparison(const std::wstring& comparison, online::LobbyFilter::ComparisonType& outComparison)
{
	if (compareIgnoreCase< std::wstring >(comparison, L"Equal") == 0)
		outComparison = online::LobbyFilter::CtEqual;
	else if (compareIgnoreCase< std::wstring >(comparison, L"NotEqual") == 0)
		outComparison = online::LobbyFilter::CtNotEqual;
	else if (compareIgnoreCase< std::wstring >(comparison, L"Less") == 0)
		outComparison = online::LobbyFilter::CtLess;
	else if (compareIgnoreCase< std::wstring >(comparison, L"LessEqual") == 0)
		outComparison = online::LobbyFilter::CtLessEqual;
	else if (compareIgnoreCase< std::wstring >(comparison, L"Greater") == 0)
		outComparison = online::LobbyFilter::CtGreater;
	else if (compareIgnoreCase< std::wstring >(comparison, L"GreaterEqual") == 0)
		outComparison = online::LobbyFilter::CtGreaterEqual;
	else
		return false;

	return true;
}

bool online_LobbyFilter_addStringComparison(online::LobbyFilter* self, const std::wstring& key, const std::wstring& value, const std::wstring& comparison)
{
	online::LobbyFilter::ComparisonType ct;
	if (!translateComparison(comparison, ct))
		return false;

	self->addComparison(key, value, ct);
	return true;
}

bool online_LobbyFilter_addNumberComparison(online::LobbyFilter* self, const std::wstring& key, int32_t value, const std::wstring& comparison)
{
	online::LobbyFilter::ComparisonType ct;
	if (!translateComparison(comparison, ct))
		return false;

	self->addComparison(key, value, ct);
	return true;
}

bool online_LobbyFilter_setDistance(online::LobbyFilter* self, const std::wstring& distance)
{
	if (compareIgnoreCase< std::wstring >(distance, L"Local"))
		self->setDistance(online::LobbyFilter::DtLocal);
	else if (compareIgnoreCase< std::wstring >(distance, L"Near"))
		self->setDistance(online::LobbyFilter::DtNear);
	else if (compareIgnoreCase< std::wstring >(distance, L"Far"))
		self->setDistance(online::LobbyFilter::DtFar);
	else if (compareIgnoreCase< std::wstring >(distance, L"Infinity"))
		self->setDistance(online::LobbyFilter::DtInfinity);
	else
		return false;

	return true;
}

std::vector< std::wstring > online_IAchievements_enumerate(online::IAchievements* self)
{
	std::set< std::wstring > achievementsIds;
	self->enumerate(achievementsIds);
	return std::vector< std::wstring >(achievementsIds.begin(), achievementsIds.end());
}

std::vector< std::wstring > online_ILeaderboards_enumerate(online::ILeaderboards* self)
{
	std::set< std::wstring > leaderboardIds;
	self->enumerate(leaderboardIds);
	return std::vector< std::wstring >(leaderboardIds.begin(), leaderboardIds.end());
}

uint32_t online_ILeaderboards_getRank(online::ILeaderboards* self, const std::wstring& leaderboardId)
{
	uint32_t rank = 0;
	self->getRank(leaderboardId, rank);
	return rank;
}

int32_t online_ILeaderboards_getScore(online::ILeaderboards* self, const std::wstring& leaderboardId)
{
	int32_t score = 0;
	self->getScore(leaderboardId, score);
	return score;
}

std::wstring online_ILobby_getMetaValue(online::ILobby* self, const std::wstring& key)
{
	std::wstring value;
	self->getMetaValue(key, value);
	return value;
}

std::wstring online_ILobby_getParticipantMetaValue(online::ILobby* self, const online::IUser* user, const std::wstring& key)
{
	std::wstring value;
	self->getParticipantMetaValue(user, key, value);
	return value;
}

std::vector< std::wstring > online_ISaveData_enumerate(online::ISaveData* self)
{
	std::set< std::wstring > saveDataIds;
	self->enumerate(saveDataIds);
	return std::vector< std::wstring >(saveDataIds.begin(), saveDataIds.end());
}

Ref< online::Result > online_ISaveData_set(online::ISaveData* self, const std::wstring& saveDataId, const std::wstring& saveDataTitle, const std::wstring& saveDataDescription, const ISerializable* attachment, bool replace)
{
	online::SaveDataDesc sdd;
	sdd.title = saveDataTitle;
	sdd.description = saveDataDescription;
	return self->set(saveDataId, sdd, attachment, replace);
}

RefArray< online::IUser > online_ISessionManager_getFriends(online::ISessionManager* self)
{
	RefArray< online::IUser > friends;
	self->getFriends(friends);
	return friends;
}

Ref< online::IUser > online_ISessionManager_findFriend(online::ISessionManager* self, const std::wstring& name)
{
	Ref< online::IUser > friendUser;
	return self->findFriend(name, friendUser) ? friendUser : 0;
}

std::vector< std::wstring > online_IStatistics_enumerate(online::IStatistics* self)
{
	std::set< std::wstring > statIds;
	self->enumerate(statIds);
	return std::vector< std::wstring >(statIds.begin(), statIds.end());
}

float online_IStatistics_get(online::IStatistics* self, const std::wstring& statId)
{
	float value = 0.0f;
	self->get(statId, value);
	return value;
}

std::wstring online_IUser_getName(online::IUser* self)
{
	std::wstring name = L"";
	self->getName(name);
	return name;
}

std::wstring online_IUser_getPresenceValue(online::IUser* self, const std::wstring& key)
{
	std::wstring value;
	self->getPresenceValue(key, value);
	return value;
}

	}

void registerOnlineClasses(script::IScriptManager* scriptManager)
{
	Ref< script::AutoScriptClass< online::Result > > classResult = new script::AutoScriptClass< online::Result >();
	classResult->addMethod(L"succeed", &online::Result::succeed);
	classResult->addMethod(L"fail", &online::Result::fail);
	classResult->addMethod(L"ready", &online::Result::ready);
	classResult->addMethod(L"succeeded", &online::Result::succeeded);
	scriptManager->registerClass(classResult);

	Ref< script::AutoScriptClass< online::AttachmentResult > > classAttachmentResult = new script::AutoScriptClass< online::AttachmentResult >();
	classAttachmentResult->addMethod(L"succeed", &online::AttachmentResult::succeed);
	classAttachmentResult->addMethod(L"get", &online::AttachmentResult::get);
	scriptManager->registerClass(classAttachmentResult);

	Ref< script::AutoScriptClass< online::LobbyResult > > classLobbyResult = new script::AutoScriptClass< online::LobbyResult >();
	classLobbyResult->addMethod(L"succeed", &online::LobbyResult::succeed);
	classLobbyResult->addMethod(L"get", &online::LobbyResult::get);
	scriptManager->registerClass(classLobbyResult);

	Ref< script::AutoScriptClass< online::LobbyArrayResult > > classLobbyArrayResult = new script::AutoScriptClass< online::LobbyArrayResult >();
	classLobbyArrayResult->addMethod(L"succeed", &online::LobbyArrayResult::succeed);
	classLobbyArrayResult->addMethod(L"get", &online::LobbyArrayResult::get);
	scriptManager->registerClass(classLobbyArrayResult);

	Ref< script::AutoScriptClass< online::UserArrayResult > > classUserArrayResult = new script::AutoScriptClass< online::UserArrayResult >();
	classUserArrayResult->addMethod(L"succeed", &online::UserArrayResult::succeed);
	classUserArrayResult->addMethod(L"get", &online::UserArrayResult::get);
	scriptManager->registerClass(classUserArrayResult);

	Ref< script::AutoScriptClass< online::LobbyFilter > > classLobbyFilter = new script::AutoScriptClass< online::LobbyFilter >();
	classLobbyFilter->addConstructor();
	classLobbyFilter->addMethod(L"addStringComparison", &online_LobbyFilter_addStringComparison);
	classLobbyFilter->addMethod(L"addNumberComparison", &online_LobbyFilter_addNumberComparison);
	classLobbyFilter->addMethod(L"setDistance", &online_LobbyFilter_setDistance);
	classLobbyFilter->addMethod(L"setSlots", &online::LobbyFilter::setSlots);
	classLobbyFilter->addMethod(L"setCount", &online::LobbyFilter::setCount);
	scriptManager->registerClass(classLobbyFilter);

	Ref< script::AutoScriptClass< online::IAchievements > > classIAchievements = new script::AutoScriptClass< online::IAchievements >();
	classIAchievements->addMethod(L"ready", &online::IAchievements::ready);
	classIAchievements->addMethod(L"enumerate", &online_IAchievements_enumerate);
	classIAchievements->addMethod(L"have", &online::IAchievements::have);
	classIAchievements->addMethod(L"set", &online::IAchievements::set);
	scriptManager->registerClass(classIAchievements);

	Ref< script::AutoScriptClass< online::ILeaderboards > > classILeaderboards = new script::AutoScriptClass< online::ILeaderboards >();
	classILeaderboards->addMethod(L"ready", &online::ILeaderboards::ready);
	classILeaderboards->addMethod(L"enumerate", &online_ILeaderboards_enumerate);
	classILeaderboards->addMethod(L"getRank", &online_ILeaderboards_getRank);
	classILeaderboards->addMethod(L"getScore", &online_ILeaderboards_getScore);
	classILeaderboards->addMethod(L"setScore", &online::ILeaderboards::setScore);
	classILeaderboards->addMethod(L"addScore", &online::ILeaderboards::addScore);
	scriptManager->registerClass(classILeaderboards);

	Ref< script::AutoScriptClass< online::ILobby > > classILobby = new script::AutoScriptClass< online::ILobby >();
	classILobby->addMethod(L"setMetaValue", &online::ILobby::setMetaValue);
	classILobby->addMethod(L"getMetaValue", &online_ILobby_getMetaValue);
	classILobby->addMethod(L"setParticipantMetaValue", &online::ILobby::setParticipantMetaValue);
	classILobby->addMethod(L"getParticipantMetaValue", &online_ILobby_getParticipantMetaValue);
	classILobby->addMethod(L"join", &online::ILobby::join);
	classILobby->addMethod(L"leave", &online::ILobby::leave);
	classILobby->addMethod(L"getParticipants", &online::ILobby::getParticipants);
	classILobby->addMethod(L"getParticipantCount", &online::ILobby::getParticipantCount);
	classILobby->addMethod(L"getIndex", &online::ILobby::getIndex);
	classILobby->addMethod(L"isOwner", &online::ILobby::isOwner);
	scriptManager->registerClass(classILobby);

	Ref< script::AutoScriptClass< online::IMatchMaking > > classIMatchMaking = new script::AutoScriptClass< online::IMatchMaking >();
	classIMatchMaking->addMethod(L"ready", &online::IMatchMaking::ready);
	classIMatchMaking->addMethod(L"findMatchingLobbies", &online::IMatchMaking::findMatchingLobbies);
	classIMatchMaking->addMethod(L"createLobby", &online::IMatchMaking::createLobby);
	scriptManager->registerClass(classIMatchMaking);

	Ref< script::AutoScriptClass< online::ISaveData > > classISaveData = new script::AutoScriptClass< online::ISaveData >();
	classISaveData->addMethod(L"ready", &online::ISaveData::ready);
	classISaveData->addMethod(L"enumerate", &online_ISaveData_enumerate);
	classISaveData->addMethod(L"get", &online::ISaveData::get);
	classISaveData->addMethod(L"set", &online_ISaveData_set);
	scriptManager->registerClass(classISaveData);

	Ref< script::AutoScriptClass< online::ISessionManager > > classISessionManager = new script::AutoScriptClass< online::ISessionManager >();
	classISessionManager->addMethod(L"destroy", &online::ISessionManager::destroy);
	classISessionManager->addMethod(L"update", &online::ISessionManager::update);
	classISessionManager->addMethod(L"getLanguageCode", &online::ISessionManager::getLanguageCode);
	classISessionManager->addMethod(L"isConnected", &online::ISessionManager::isConnected);
	classISessionManager->addMethod(L"requireUserAttention", &online::ISessionManager::requireUserAttention);
	classISessionManager->addMethod(L"getFriends", &online_ISessionManager_getFriends);
	classISessionManager->addMethod(L"findFriend", &online_ISessionManager_findFriend);
	classISessionManager->addMethod(L"haveP2PData", &online::ISessionManager::haveP2PData);
	classISessionManager->addMethod(L"getAchievements", &online::ISessionManager::getAchievements);
	classISessionManager->addMethod(L"getLeaderboards", &online::ISessionManager::getLeaderboards);
	classISessionManager->addMethod(L"getMatchMaking", &online::ISessionManager::getMatchMaking);
	classISessionManager->addMethod(L"getSaveData", &online::ISessionManager::getSaveData);
	classISessionManager->addMethod(L"getStatistics", &online::ISessionManager::getStatistics);
	classISessionManager->addMethod(L"getUser", &online::ISessionManager::getUser);
	scriptManager->registerClass(classISessionManager);

	Ref< script::AutoScriptClass< online::IStatistics > > classIStatistics = new script::AutoScriptClass< online::IStatistics >();
	classIStatistics->addMethod(L"ready", &online::IStatistics::ready);
	classIStatistics->addMethod(L"enumerate", &online_IStatistics_enumerate);
	classIStatistics->addMethod(L"get", &online_IStatistics_get);
	classIStatistics->addMethod(L"set", &online::IStatistics::set);
	classIStatistics->addMethod(L"add", &online::IStatistics::add);
	scriptManager->registerClass(classIStatistics);

	Ref< script::AutoScriptClass< online::IUser > > classIUser = new script::AutoScriptClass< online::IUser >();
	classIUser->addMethod(L"getName", &online_IUser_getName);
	classIUser->addMethod(L"isFriend", &online::IUser::isFriend);
	classIUser->addMethod(L"invite", &online::IUser::invite);
	classIUser->addMethod(L"setPresenceValue", &online::IUser::setPresenceValue);
	classIUser->addMethod(L"getPresenceValue", &online_IUser_getPresenceValue);
	scriptManager->registerClass(classIUser);
}

	}
}
