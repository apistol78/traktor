/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/Boxes/BoxedRefArray.h"
#include "Core/Class/Boxes/BoxedStdVector.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyGroup.h"
#include "Drawing/Image.h"
#include "Net/Url.h"
#include "Online/OnlineClassFactory.h"
#include "Online/IAchievements.h"
#include "Online/ILeaderboards.h"
#include "Online/ILobby.h"
#include "Online/IMatchMaking.h"
#include "Online/IParty.h"
#include "Online/ISaveData.h"
#include "Online/ISessionManager.h"
#include "Online/IStatistics.h"
#include "Online/IUser.h"
#include "Online/IVideoSharing.h"
#include "Online/IVoiceChat.h"
#include "Online/LobbyFilter.h"
#include "Online/OnlinePeer2PeerProvider.h"
#include "Online/Score.h"
#include "Online/ScoreArrayResult.h"
#include "Sound/Player/ISoundPlayer.h"

namespace traktor
{
	namespace online
	{
		namespace
		{

void LobbyFilter_addComparison(LobbyFilter* self, const std::wstring& key, const Any& value, int32_t comparison)
{
	self->addComparison(key, value.getInt32Unsafe(), (LobbyFilter::Comparison)comparison);
}

void LobbyFilter_setDistance(LobbyFilter* self, int32_t distance)
{
	self->setDistance((LobbyFilter::Distance)distance);
}

int32_t LobbyFilter_getDistance(LobbyFilter* self)
{
	return (int32_t)self->getDistance();
}

std::vector< std::wstring > IAchievements_enumerate(IAchievements* self)
{
	std::set< std::wstring > achievementsIds;
	self->enumerate(achievementsIds);
	return std::vector< std::wstring >(achievementsIds.begin(), achievementsIds.end());
}

std::vector< std::wstring > ILeaderboards_enumerate(ILeaderboards* self)
{
	std::set< std::wstring > leaderboardIds;
	self->enumerate(leaderboardIds);
	return std::vector< std::wstring >(leaderboardIds.begin(), leaderboardIds.end());
}

uint32_t ILeaderboards_getRank(ILeaderboards* self, const std::wstring& leaderboardId)
{
	uint32_t rank = 0;
	self->getRank(leaderboardId, rank);
	return rank;
}

int32_t ILeaderboards_getScore(ILeaderboards* self, const std::wstring& leaderboardId)
{
	int32_t score = 0;
	self->getScore(leaderboardId, score);
	return score;
}

std::wstring ILobby_getMetaValue(ILobby* self, const std::wstring& key)
{
	std::wstring value;
	self->getMetaValue(key, value);
	return value;
}

std::wstring ILobby_getParticipantMetaValue(ILobby* self, const IUser* user, const std::wstring& key)
{
	std::wstring value;
	self->getParticipantMetaValue(user, key, value);
	return value;
}

RefArray< IUser > ILobby_getParticipants(ILobby* self)
{
	RefArray< IUser > users;
	self->getParticipants(users);
	return users;
}

void ILobby_setOwner(ILobby* self, const IUser* user)
{
	self->setOwner(user);
}

const IUser* ILobby_getOwner(ILobby* self)
{
	return self->getOwner();
}

Ref< LobbyResult > IMatchMaking_createLobby(IMatchMaking* self, uint32_t maxUsers, const std::wstring& access)
{
	LobbyAccess la;

	if (compareIgnoreCase(access, L"public") == 0)
		la = LaPublic;
	else if (compareIgnoreCase(access, L"private") == 0)
		la = LaPrivate;
	else if (compareIgnoreCase(access, L"friends") == 0)
		la = LaFriends;
	else
		return 0;

	return self->createLobby(maxUsers, la);
}

std::wstring IParty_getMetaValue(IParty* self, const std::wstring& key)
{
	std::wstring value;
	self->getMetaValue(key, value);
	return value;
}

std::wstring IParty_getParticipantMetaValue(IParty* self, const IUser* user, const std::wstring& key)
{
	std::wstring value;
	self->getParticipantMetaValue(user, key, value);
	return value;
}

RefArray< IUser > IParty_getParticipants(IParty* self)
{
	RefArray< IUser > users;
	self->getParticipants(users);
	return users;
}

std::vector< std::wstring > ISaveData_enumerate(ISaveData* self)
{
	std::set< std::wstring > saveDataIds;
	self->enumerate(saveDataIds);
	return std::vector< std::wstring >(saveDataIds.begin(), saveDataIds.end());
}

Ref< Result > ISaveData_set(ISaveData* self, const std::wstring& saveDataId, const std::wstring& saveDataTitle, const std::wstring& saveDataDescription, const ISerializable* attachment, bool replace)
{
	SaveDataDesc sdd;
	sdd.title = saveDataTitle;
	sdd.description = saveDataDescription;
	return self->set(saveDataId, sdd, attachment, replace);
}

bool ISaveData_setNow(ISaveData* self, const std::wstring& saveDataId, const std::wstring& saveDataTitle, const std::wstring& saveDataDescription, const ISerializable* attachment, bool replace)
{
	SaveDataDesc sdd;
	sdd.title = saveDataTitle;
	sdd.description = saveDataDescription;
	return self->setNow(saveDataId, sdd, attachment, replace);
}

bool ISessionManager_navigateUrl(ISessionManager* self, const std::wstring& url)
{
	return self->navigateUrl(net::Url(url));
}

RefArray< IUser > ISessionManager_getFriends(ISessionManager* self, bool onlineOnly)
{
	RefArray< IUser > friends;
	self->getFriends(friends, onlineOnly);
	return friends;
}

Ref< IUser > ISessionManager_findFriend(ISessionManager* self, const std::wstring& name)
{
	Ref< IUser > friendUser;
	return self->findFriend(name, friendUser) ? friendUser : 0;
}

std::vector< std::wstring > IStatistics_enumerate(IStatistics* self)
{
	std::set< std::wstring > statIds;
	self->enumerate(statIds);
	return std::vector< std::wstring >(statIds.begin(), statIds.end());
}

int32_t IStatistics_get(IStatistics* self, const std::wstring& statId)
{
	int32_t value = 0;
	self->get(statId, value);
	return value;
}

std::wstring IUser_getName(IUser* self)
{
	std::wstring name = L"";
	self->getName(name);
	return name;
}

std::wstring IUser_getPresenceValue(IUser* self, const std::wstring& key)
{
	std::wstring value;
	self->getPresenceValue(key, value);
	return value;
}

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.online.OnlineClassFactory", 0, OnlineClassFactory, IRuntimeClassFactory)

void OnlineClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	auto classAttachmentResult = new AutoRuntimeClass< AttachmentResult >();
	classAttachmentResult->addProperty("attachment", &AttachmentResult::get);
	classAttachmentResult->addMethod("succeed", &AttachmentResult::succeed);
	registrar->registerClass(classAttachmentResult);

	auto classLobbyResult = new AutoRuntimeClass< LobbyResult >();
	classLobbyResult->addProperty("lobby", &LobbyResult::get);
	classLobbyResult->addMethod("succeed", &LobbyResult::succeed);
	registrar->registerClass(classLobbyResult);

	auto classLobbyArrayResult = new AutoRuntimeClass< LobbyArrayResult >();
	classLobbyArrayResult->addProperty("lobbies", &LobbyArrayResult::get);
	classLobbyArrayResult->addMethod("succeed", &LobbyArrayResult::succeed);
	registrar->registerClass(classLobbyArrayResult);

	auto classPartyResult = new AutoRuntimeClass< PartyResult >();
	classPartyResult->addProperty("party", &PartyResult::get);
	classPartyResult->addMethod("succeed", &PartyResult::succeed);
	registrar->registerClass(classPartyResult);

	auto classScore = new AutoRuntimeClass< Score >();
	classScore->addProperty("user", &Score::getUser);
	classScore->addProperty("score", &Score::getScore);
	classScore->addProperty("rank", &Score::getRank);
	registrar->registerClass(classScore);

	auto classScoreArrayResult = new AutoRuntimeClass< ScoreArrayResult >();
	classScoreArrayResult->addMethod("succeed", &ScoreArrayResult::succeed);
	classScoreArrayResult->addProperty("scores", &ScoreArrayResult::get);
	registrar->registerClass(classScoreArrayResult);

	auto classUserArrayResult = new AutoRuntimeClass< UserArrayResult >();
	classUserArrayResult->addMethod("succeed", &UserArrayResult::succeed);
	classUserArrayResult->addProperty("users", &UserArrayResult::get);
	registrar->registerClass(classUserArrayResult);

	auto classLobbyFilter = new AutoRuntimeClass< LobbyFilter >();
	classLobbyFilter->addConstant("Equal", Any::fromInt32((int32_t)LobbyFilter::Comparison::Equal));
	classLobbyFilter->addConstant("NotEqual", Any::fromInt32((int32_t)LobbyFilter::Comparison::NotEqual));
	classLobbyFilter->addConstant("Less", Any::fromInt32((int32_t)LobbyFilter::Comparison::Less));
	classLobbyFilter->addConstant("LessEqual", Any::fromInt32((int32_t)LobbyFilter::Comparison::LessEqual));
	classLobbyFilter->addConstant("Greater", Any::fromInt32((int32_t)LobbyFilter::Comparison::Greater));
	classLobbyFilter->addConstant("GreaterEqual", Any::fromInt32((int32_t)LobbyFilter::Comparison::GreaterEqual));
	classLobbyFilter->addConstant("Unspecified", Any::fromInt32((int32_t)LobbyFilter::Distance::Unspecified));
	classLobbyFilter->addConstant("Local", Any::fromInt32((int32_t)LobbyFilter::Distance::Local));
	classLobbyFilter->addConstant("Near", Any::fromInt32((int32_t)LobbyFilter::Distance::Near));
	classLobbyFilter->addConstant("Far", Any::fromInt32((int32_t)LobbyFilter::Distance::Far));
	classLobbyFilter->addConstant("Infinity", Any::fromInt32((int32_t)LobbyFilter::Distance::Infinity));
	classLobbyFilter->addProperty("distance", &LobbyFilter_setDistance, &LobbyFilter_getDistance);
	classLobbyFilter->addProperty("slots", &LobbyFilter::setSlots, &LobbyFilter::getSlots);
	classLobbyFilter->addProperty("count", &LobbyFilter::setCount, &LobbyFilter::getCount);
	classLobbyFilter->addConstructor();
	classLobbyFilter->addMethod("addComparison", &LobbyFilter_addComparison);
	registrar->registerClass(classLobbyFilter);

	auto classIAchievements = new AutoRuntimeClass< IAchievements >();
	classIAchievements->addProperty("ready", &IAchievements::ready);
	classIAchievements->addMethod("enumerate", &IAchievements_enumerate);
	classIAchievements->addMethod("have", &IAchievements::have);
	classIAchievements->addMethod("set", &IAchievements::set);
	registrar->registerClass(classIAchievements);

	auto classILeaderboards = new AutoRuntimeClass< ILeaderboards >();
	classILeaderboards->addProperty("ready", &ILeaderboards::ready);
	classILeaderboards->addMethod("enumerate", &ILeaderboards_enumerate);
	classILeaderboards->addMethod("create", &ILeaderboards::create);
	classILeaderboards->addMethod("getRank", &ILeaderboards_getRank);
	classILeaderboards->addMethod("getScore", &ILeaderboards_getScore);
	classILeaderboards->addMethod("setScore", &ILeaderboards::setScore);
	classILeaderboards->addMethod("addScore", &ILeaderboards::addScore);
	classILeaderboards->addMethod("getGlobalScores", &ILeaderboards::getGlobalScores);
	classILeaderboards->addMethod("getFriendScores", &ILeaderboards::getFriendScores);
	registrar->registerClass(classILeaderboards);

	auto classILobby = new AutoRuntimeClass< ILobby >();
	classILobby->addProperty("participants", &ILobby_getParticipants);
	classILobby->addProperty("participantCount", &ILobby::getParticipantCount);
	classILobby->addProperty("maxParticipantCount", &ILobby::getMaxParticipantCount);
	classILobby->addProperty("friendsCount", &ILobby::getFriendsCount);
	classILobby->addProperty("owner", &ILobby_setOwner, &ILobby_getOwner);
	classILobby->addMethod("setMetaValue", &ILobby::setMetaValue);
	classILobby->addMethod("getMetaValue", &ILobby_getMetaValue);
	classILobby->addMethod("setParticipantMetaValue", &ILobby::setParticipantMetaValue);
	classILobby->addMethod("getParticipantMetaValue", &ILobby_getParticipantMetaValue);
	classILobby->addMethod("join", &ILobby::join);
	classILobby->addMethod("leave", &ILobby::leave);
	classILobby->addMethod("invite", &ILobby::invite);
	registrar->registerClass(classILobby);

	auto classIMatchMaking = new AutoRuntimeClass< IMatchMaking >();
	classIMatchMaking->addProperty("ready", &IMatchMaking::ready);
	classIMatchMaking->addMethod("findMatchingLobbies", &IMatchMaking::findMatchingLobbies);
	classIMatchMaking->addMethod("createLobby", &IMatchMaking_createLobby);
	classIMatchMaking->addMethod("acceptLobby", &IMatchMaking::acceptLobby);
	classIMatchMaking->addMethod("createParty", &IMatchMaking::createParty);
	classIMatchMaking->addMethod("acceptParty", &IMatchMaking::acceptParty);
	registrar->registerClass(classIMatchMaking);

	auto classIParty = new AutoRuntimeClass< IParty >();
	classIParty->addProperty("participants", &IParty_getParticipants);
	classIParty->addProperty("participantCount", &IParty::getParticipantCount);
	classIParty->addMethod("setMetaValue", &IParty::setMetaValue);
	classIParty->addMethod("getMetaValue", &IParty_getMetaValue);
	classIParty->addMethod("setParticipantMetaValue", &IParty::setParticipantMetaValue);
	classIParty->addMethod("getParticipantMetaValue", &IParty_getParticipantMetaValue);
	classIParty->addMethod("leave", &IParty::leave);
	classIParty->addMethod("invite", &IParty::invite);
	registrar->registerClass(classIParty);

	auto classISaveData = new AutoRuntimeClass< ISaveData >();
	classISaveData->addProperty("ready", &ISaveData::ready);
	classISaveData->addMethod("enumerate", &ISaveData_enumerate);
	classISaveData->addMethod("get", &ISaveData::get);
	classISaveData->addMethod("getNow", &ISaveData::getNow);
	classISaveData->addMethod("set", &ISaveData_set);
	classISaveData->addMethod("setNow", &ISaveData_setNow);
	classISaveData->addMethod("remove", &ISaveData::remove);
	registrar->registerClass(classISaveData);

	auto classISessionManager = new AutoRuntimeClass< ISessionManager >();
	classISessionManager->addProperty("languageCode", &ISessionManager::getLanguageCode);
	classISessionManager->addProperty("connected", &ISessionManager::isConnected);
	classISessionManager->addProperty("haveP2PData", &ISessionManager::haveP2PData);
	classISessionManager->addProperty("requireFullScreen", &ISessionManager::requireFullScreen);
	classISessionManager->addProperty("requireUserAttention", &ISessionManager::requireUserAttention);
	classISessionManager->addProperty("currentGameCount", &ISessionManager::getCurrentGameCount);
	classISessionManager->addProperty("achievements", &ISessionManager::getAchievements);
	classISessionManager->addProperty("leaderboards", &ISessionManager::getLeaderboards);
	classISessionManager->addProperty("matchMaking", &ISessionManager::getMatchMaking);
	classISessionManager->addProperty("saveData", &ISessionManager::getSaveData);
	classISessionManager->addProperty("statistics", &ISessionManager::getStatistics);
	classISessionManager->addProperty("user", &ISessionManager::getUser);
	classISessionManager->addProperty("videoSharing", &ISessionManager::getVideoSharing);
	classISessionManager->addProperty("voiceChat", &ISessionManager::getVoiceChat);
	classISessionManager->addMethod("destroy", &ISessionManager::destroy);
	classISessionManager->addMethod("update", &ISessionManager::update);
	classISessionManager->addMethod("haveDLC", &ISessionManager::haveDLC);
	classISessionManager->addMethod("buyDLC", &ISessionManager::buyDLC);
	classISessionManager->addMethod("overrideDLC", &ISessionManager::overrideDLC);
	classISessionManager->addMethod("navigateUrl", &ISessionManager_navigateUrl);
	classISessionManager->addMethod("getFriends", &ISessionManager_getFriends);
	classISessionManager->addMethod("findFriend", &ISessionManager_findFriend);
	registrar->registerClass(classISessionManager);

	auto classIStatistics = new AutoRuntimeClass< IStatistics >();
	classIStatistics->addProperty("ready", &IStatistics::ready);
	classIStatistics->addMethod("enumerate", &IStatistics_enumerate);
	classIStatistics->addMethod("get", &IStatistics_get);
	classIStatistics->addMethod("set", &IStatistics::set);
	classIStatistics->addMethod("add", &IStatistics::add);
	registrar->registerClass(classIStatistics);

	auto classIUser = new AutoRuntimeClass< IUser >();
	classIUser->addProperty("name", &IUser_getName);
	classIUser->addProperty("image", &IUser::getImage);
	classIUser->addProperty("globalId", &IUser::getGlobalId);
	classIUser->addProperty("friend", &IUser::isFriend);
	classIUser->addProperty("p2pAllowed", &IUser::isP2PAllowed);
	classIUser->addProperty("p2pRelayed", &IUser::isP2PRelayed);
	classIUser->addMethod("isMemberOfGroup", &IUser::isMemberOfGroup);
	classIUser->addMethod("joinGroup", &IUser::joinGroup);
	classIUser->addMethod("invite", &IUser::invite);
	classIUser->addMethod("setPresenceValue", &IUser::setPresenceValue);
	classIUser->addMethod("getPresenceValue", &IUser_getPresenceValue);
	classIUser->addMethod("setP2PEnable", &IUser::setP2PEnable);
	registrar->registerClass(classIUser);

	auto classIVideoSharing = new AutoRuntimeClass< IVideoSharing >();
	classIVideoSharing->addProperty("capturing", &IVideoSharing::isCapturing);
	classIVideoSharing->addMethod("beginCapture", &IVideoSharing::beginCapture);
	classIVideoSharing->addMethod("endCapture", &IVideoSharing::endCapture);
	classIVideoSharing->addMethod("showShareUI", &IVideoSharing::showShareUI);
	registrar->registerClass(classIVideoSharing);

	auto classIVoiceChat = new AutoRuntimeClass< IVoiceChat >();
	classIVoiceChat->addMethod("setSoundPlayer", &IVoiceChat::setSoundPlayer);
	classIVoiceChat->addMethod("setAudience", &IVoiceChat::setAudience);
	classIVoiceChat->addMethod("beginTransmission", &IVoiceChat::beginTransmission);
	classIVoiceChat->addMethod("endTransmission", &IVoiceChat::endTransmission);
	classIVoiceChat->addMethod("setMute", &IVoiceChat::setMute);
	classIVoiceChat->addMethod("isTransmitting", &IVoiceChat::isTransmitting);
	registrar->registerClass(classIVoiceChat);

	auto classOnlinePeer2PeerProvider = new AutoRuntimeClass< OnlinePeer2PeerProvider >();
	classOnlinePeer2PeerProvider->addConstructor< ISessionManager*, ILobby*, bool, bool >();
	registrar->registerClass(classOnlinePeer2PeerProvider);
}

	}
}
