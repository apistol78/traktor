/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/Boxes.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Core/Class/IRuntimeDelegate.h"
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
#include "Sound/Player/ISoundPlayer.h"

namespace traktor
{
	namespace online
	{
		namespace
		{

class ResultDeferred : public Result::IDeferred
{
public:
	ResultDeferred(IRuntimeDelegate* delegate)
	:	m_delegate(delegate)
	{
	}

	virtual void dispatch(const Result& result) const
	{
		const Any argv[] =
		{
			Any::fromObject(const_cast< Result* >(&result))
		};
		m_delegate->call(sizeof_array(argv), argv);
	}

private:
	Ref< IRuntimeDelegate > m_delegate;
};

void Result_defer(Result* self, IRuntimeDelegate* delegate)
{
	self->defer(new ResultDeferred(delegate));
}

bool translateComparison(const std::wstring& comparison, LobbyFilter::ComparisonType& outComparison)
{
	const struct { const wchar_t* id; LobbyFilter::ComparisonType value; } c_comparisons[] =
	{
		{ L"Equal", LobbyFilter::CtEqual },
		{ L"NotEqual", LobbyFilter::CtNotEqual },
		{ L"Less", LobbyFilter::CtLess },
		{ L"LessEqual", LobbyFilter::CtLessEqual },
		{ L"Greater", LobbyFilter::CtGreater },
		{ L"GreaterEqual", LobbyFilter::CtGreaterEqual }
	};

	for (uint32_t i = 0; i < sizeof_array(c_comparisons); ++i)
	{
		if (compareIgnoreCase< std::wstring >(comparison, c_comparisons[i].id) == 0)
		{
			outComparison = c_comparisons[i].value;
			return true;
		}
	}

	return false;
}

bool LobbyFilter_addComparison(LobbyFilter* self, const std::wstring& key, const Any& value, int32_t comparison)
{
	if (value.isInt32())
	{
		self->addComparison(key, value.getInt32Unsafe(), (LobbyFilter::ComparisonType)comparison);
		return true;
	}
	else if (value.isString())
	{
		self->addComparison(key, value.getWideString(), (LobbyFilter::ComparisonType)comparison);
		return true;
	}
	else
		return false;
}

bool LobbyFilter_setDistance(LobbyFilter* self, const std::wstring& distance)
{
	if (compareIgnoreCase< std::wstring >(distance, L"Local"))
		self->setDistance(LobbyFilter::DtLocal);
	else if (compareIgnoreCase< std::wstring >(distance, L"Near"))
		self->setDistance(LobbyFilter::DtNear);
	else if (compareIgnoreCase< std::wstring >(distance, L"Far"))
		self->setDistance(LobbyFilter::DtFar);
	else if (compareIgnoreCase< std::wstring >(distance, L"Infinity"))
		self->setDistance(LobbyFilter::DtInfinity);
	else
		return false;

	return true;
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

	if (compareIgnoreCase< std::wstring >(access, L"public") == 0)
		la = LaPublic;
	else if (compareIgnoreCase< std::wstring >(access, L"private") == 0)
		la = LaPrivate;
	else if (compareIgnoreCase< std::wstring >(access, L"friends") == 0)
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
	Ref< AutoRuntimeClass< Result > > classResult = new AutoRuntimeClass< Result >();
	classResult->addProperty("ready", &Result::ready);
	classResult->addProperty("succeeded", &Result::succeeded);
	classResult->addMethod("succeed", &Result::succeed);
	classResult->addMethod("fail", &Result::fail);
	classResult->addMethod("defer", &Result_defer);
	classResult->addMethod("wait", &Result::succeeded);	// Mapping wait to succeeded, will block until result is available.
	registrar->registerClass(classResult);

	Ref< AutoRuntimeClass< AttachmentResult > > classAttachmentResult = new AutoRuntimeClass< AttachmentResult >();
	classAttachmentResult->addProperty("attachment", &AttachmentResult::get);
	classAttachmentResult->addMethod("succeed", &AttachmentResult::succeed);
	registrar->registerClass(classAttachmentResult);

	Ref< AutoRuntimeClass< LobbyResult > > classLobbyResult = new AutoRuntimeClass< LobbyResult >();
	classLobbyResult->addProperty("lobby", &LobbyResult::get);
	classLobbyResult->addMethod("succeed", &LobbyResult::succeed);
	registrar->registerClass(classLobbyResult);

	Ref< AutoRuntimeClass< LobbyArrayResult > > classLobbyArrayResult = new AutoRuntimeClass< LobbyArrayResult >();
	classLobbyArrayResult->addProperty("lobbies", &LobbyArrayResult::get);
	classLobbyArrayResult->addMethod("succeed", &LobbyArrayResult::succeed);
	registrar->registerClass(classLobbyArrayResult);

	Ref< AutoRuntimeClass< PartyResult > > classPartyResult = new AutoRuntimeClass< PartyResult >();
	classPartyResult->addProperty("party", &PartyResult::get);
	classPartyResult->addMethod("succeed", &PartyResult::succeed);
	registrar->registerClass(classPartyResult);

	Ref< AutoRuntimeClass< Score > > classScore = new AutoRuntimeClass< Score >();
	classScore->addProperty("user", &Score::getUser);
	classScore->addProperty("score", &Score::getScore);
	classScore->addProperty("rank", &Score::getRank);
	registrar->registerClass(classScore);

	Ref< AutoRuntimeClass< ScoreArrayResult > > classScoreArrayResult = new AutoRuntimeClass< ScoreArrayResult >();
	classScoreArrayResult->addMethod("succeed", &ScoreArrayResult::succeed);
	classScoreArrayResult->addProperty("scores", &ScoreArrayResult::get);
	registrar->registerClass(classScoreArrayResult);

	Ref< AutoRuntimeClass< UserArrayResult > > classUserArrayResult = new AutoRuntimeClass< UserArrayResult >();
	classUserArrayResult->addMethod("succeed", &UserArrayResult::succeed);
	classUserArrayResult->addProperty("users", &UserArrayResult::get);
	registrar->registerClass(classUserArrayResult);

	Ref< AutoRuntimeClass< LobbyFilter > > classLobbyFilter = new AutoRuntimeClass< LobbyFilter >();
	classLobbyFilter->addConstant("CtEqual", Any::fromInt32(LobbyFilter::CtEqual));
	classLobbyFilter->addConstant("CtNotEqual", Any::fromInt32(LobbyFilter::CtNotEqual));
	classLobbyFilter->addConstant("CtLess", Any::fromInt32(LobbyFilter::CtLess));
	classLobbyFilter->addConstant("CtLessEqual", Any::fromInt32(LobbyFilter::CtLessEqual));
	classLobbyFilter->addConstant("CtGreater", Any::fromInt32(LobbyFilter::CtGreater));
	classLobbyFilter->addConstant("CtGreaterEqual", Any::fromInt32(LobbyFilter::CtGreaterEqual));
	classLobbyFilter->addConstructor();
	classLobbyFilter->addMethod("addComparison", &LobbyFilter_addComparison);
	classLobbyFilter->addMethod("setDistance", &LobbyFilter_setDistance);
	classLobbyFilter->addMethod("setSlots", &LobbyFilter::setSlots);
	classLobbyFilter->addMethod("setCount", &LobbyFilter::setCount);
	registrar->registerClass(classLobbyFilter);

	Ref< AutoRuntimeClass< IAchievements > > classIAchievements = new AutoRuntimeClass< IAchievements >();
	classIAchievements->addProperty("ready", &IAchievements::ready);
	classIAchievements->addMethod("enumerate", &IAchievements_enumerate);
	classIAchievements->addMethod("have", &IAchievements::have);
	classIAchievements->addMethod("set", &IAchievements::set);
	registrar->registerClass(classIAchievements);

	Ref< AutoRuntimeClass< ILeaderboards > > classILeaderboards = new AutoRuntimeClass< ILeaderboards >();
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

	Ref< AutoRuntimeClass< ILobby > > classILobby = new AutoRuntimeClass< ILobby >();
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

	Ref< AutoRuntimeClass< IMatchMaking > > classIMatchMaking = new AutoRuntimeClass< IMatchMaking >();
	classIMatchMaking->addProperty("ready", &IMatchMaking::ready);
	classIMatchMaking->addMethod("findMatchingLobbies", &IMatchMaking::findMatchingLobbies);
	classIMatchMaking->addMethod("createLobby", &IMatchMaking_createLobby);
	classIMatchMaking->addMethod("acceptLobby", &IMatchMaking::acceptLobby);
	classIMatchMaking->addMethod("createParty", &IMatchMaking::createParty);
	classIMatchMaking->addMethod("acceptParty", &IMatchMaking::acceptParty);
	registrar->registerClass(classIMatchMaking);

	Ref< AutoRuntimeClass< IParty > > classIParty = new AutoRuntimeClass< IParty >();
	classIParty->addProperty("participants", &IParty_getParticipants);
	classIParty->addProperty("participantCount", &IParty::getParticipantCount);
	classIParty->addMethod("setMetaValue", &IParty::setMetaValue);
	classIParty->addMethod("getMetaValue", &IParty_getMetaValue);
	classIParty->addMethod("setParticipantMetaValue", &IParty::setParticipantMetaValue);
	classIParty->addMethod("getParticipantMetaValue", &IParty_getParticipantMetaValue);
	classIParty->addMethod("leave", &IParty::leave);
	classIParty->addMethod("invite", &IParty::invite);
	registrar->registerClass(classIParty);

	Ref< AutoRuntimeClass< ISaveData > > classISaveData = new AutoRuntimeClass< ISaveData >();
	classISaveData->addProperty("ready", &ISaveData::ready);
	classISaveData->addMethod("enumerate", &ISaveData_enumerate);
	classISaveData->addMethod("get", &ISaveData::get);
	classISaveData->addMethod("getNow", &ISaveData::getNow);
	classISaveData->addMethod("set", &ISaveData_set);
	classISaveData->addMethod("setNow", &ISaveData_setNow);
	classISaveData->addMethod("remove", &ISaveData::remove);
	registrar->registerClass(classISaveData);

	Ref< AutoRuntimeClass< ISessionManager > > classISessionManager = new AutoRuntimeClass< ISessionManager >();
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
	classISessionManager->addMethod("navigateUrl", &ISessionManager_navigateUrl);
	classISessionManager->addMethod("getFriends", &ISessionManager_getFriends);
	classISessionManager->addMethod("findFriend", &ISessionManager_findFriend);
	registrar->registerClass(classISessionManager);

	Ref< AutoRuntimeClass< IStatistics > > classIStatistics = new AutoRuntimeClass< IStatistics >();
	classIStatistics->addProperty("ready", &IStatistics::ready);
	classIStatistics->addMethod("enumerate", &IStatistics_enumerate);
	classIStatistics->addMethod("get", &IStatistics_get);
	classIStatistics->addMethod("set", &IStatistics::set);
	classIStatistics->addMethod("add", &IStatistics::add);
	registrar->registerClass(classIStatistics);

	Ref< AutoRuntimeClass< IUser > > classIUser = new AutoRuntimeClass< IUser >();
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

	Ref< AutoRuntimeClass< IVideoSharing > > classIVideoSharing = new AutoRuntimeClass< IVideoSharing >();
	classIVideoSharing->addProperty("capturing", &IVideoSharing::isCapturing);
	classIVideoSharing->addMethod("beginCapture", &IVideoSharing::beginCapture);
	classIVideoSharing->addMethod("endCapture", &IVideoSharing::endCapture);
	classIVideoSharing->addMethod("showShareUI", &IVideoSharing::showShareUI);
	registrar->registerClass(classIVideoSharing);

	Ref< AutoRuntimeClass< IVoiceChat > > classIVoiceChat = new AutoRuntimeClass< IVoiceChat >();
	classIVoiceChat->addMethod("setSoundPlayer", &IVoiceChat::setSoundPlayer);
	classIVoiceChat->addMethod("setAudience", &IVoiceChat::setAudience);
	classIVoiceChat->addMethod("beginTransmission", &IVoiceChat::beginTransmission);
	classIVoiceChat->addMethod("endTransmission", &IVoiceChat::endTransmission);
	classIVoiceChat->addMethod("setMute", &IVoiceChat::setMute);
	classIVoiceChat->addMethod("isTransmitting", &IVoiceChat::isTransmitting);
	registrar->registerClass(classIVoiceChat);

	Ref< AutoRuntimeClass< OnlinePeer2PeerProvider > > classOnlinePeer2PeerProvider = new AutoRuntimeClass< OnlinePeer2PeerProvider >();
	classOnlinePeer2PeerProvider->addConstructor< ISessionManager*, ILobby*, bool, bool >();
	registrar->registerClass(classOnlinePeer2PeerProvider);
}

	}
}
