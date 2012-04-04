#include "Core/Serialization/ISerializable.h"
#include "Online/IAchievements.h"
#include "Online/ILeaderboards.h"
#include "Online/ILobby.h"
#include "Online/IMatchMaking.h"
#include "Online/ISaveData.h"
#include "Online/ISessionManager.h"
#include "Online/IStatistics.h"
#include "Online/IUser.h"
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
	std::wstring name = L"N/A";
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
	scriptManager->registerClass(classILeaderboards);

	Ref< script::AutoScriptClass< online::ILobby > > classILobby = new script::AutoScriptClass< online::ILobby >();
	classILobby->addMethod(L"setMetaValue", &online::ILobby::setMetaValue);
	classILobby->addMethod(L"getMetaValue", &online_ILobby_getMetaValue);
	classILobby->addMethod(L"setParticipantMetaValue", &online::ILobby::setParticipantMetaValue);
	classILobby->addMethod(L"getParticipantMetaValue", &online_ILobby_getParticipantMetaValue);
	classILobby->addMethod(L"join", &online::ILobby::join);
	classILobby->addMethod(L"leave", &online::ILobby::leave);
	classILobby->addMethod(L"getParticipants", &online::ILobby::getParticipants);
	classILobby->addMethod(L"getIndex", &online::ILobby::getIndex);
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
	classIUser->addMethod(L"setPresenceValue", &online::IUser::setPresenceValue);
	classIUser->addMethod(L"getPresenceValue", &online_IUser_getPresenceValue);
	scriptManager->registerClass(classIUser);
}

	}
}
