/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Net/SocketAddressIPv4.h"
#include "Net/UdpSocket.h"
#include "Net/Discovery/DiscoveryManager.h"
#include "Net/Discovery/NetworkService.h"
#include "Online/Lan/LanAchievements.h"
#include "Online/Lan/LanLeaderboards.h"
#include "Online/Lan/LanMatchMaking.h"
#include "Online/Lan/LanSaveData.h"
#include "Online/Lan/LanSessionManager.h"
#include "Online/Lan/LanStatistics.h"
#include "Online/Lan/LanUser.h"
#include "Online/Lan/LanVoiceChat.h"

namespace traktor
{
	namespace online
	{
		namespace
		{

const wchar_t* c_keyServiceTypeUser = L"U";
const wchar_t* c_keyUserHandle = L"UH";
const wchar_t* c_keyUserAddr = L"UA";
const wchar_t* c_keyUserPort = L"UP";

int32_t generateUniqueKey()
{
	Guid unique = Guid::create();
	const uint8_t* p = unique;
	int32_t k = *(const int32_t*)&p[0] ^ *(const int32_t*)&p[4] ^ *(const int32_t*)&p[8] ^ *(const int32_t*)&p[12];
	if (k < 0) k = -k;
	return k;
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.online.LanSessionManager", 0, LanSessionManager, ISessionManagerProvider)

LanSessionManager::LanSessionManager()
:	m_userHandle(0)
,	m_port(0)
{
}

bool LanSessionManager::create(const IGameConfiguration* configuration)
{
	m_discoveryManager = new net::DiscoveryManager();
	if (!m_discoveryManager->create(net::MdFindServices | net::MdPublishServices/* | net::MdVerbose*/))
		return false;

	m_userHandle = generateUniqueKey();

	// Create our UDP receiving socket.
	net::SocketAddressIPv4::Interface itf;
	if (!net::SocketAddressIPv4::getBestInterface(itf))
		return false;

	m_socket = new net::UdpSocket();
	if (!m_socket->bind(net::SocketAddressIPv4(itf.addr->getAddr(), 0)))
		return false;

	m_port = dynamic_type_cast< net::SocketAddressIPv4* >(m_socket->getLocalAddress())->getPort();
	log::info << L"Listening to port " << m_port << Endl;

	// Expose our self on the network.
	Ref< PropertyGroup > propertyGroup = new PropertyGroup();
	propertyGroup->setProperty< PropertyInteger >(c_keyUserHandle, m_userHandle);
	propertyGroup->setProperty< PropertyInteger >(c_keyUserAddr, itf.addr->getAddr());
	propertyGroup->setProperty< PropertyInteger >(c_keyUserPort, m_port);
	m_discoveryManager->addService(new net::NetworkService(c_keyServiceTypeUser, propertyGroup));

	// Create implementations of online systems.
	m_achievements = new LanAchievements();
	m_leaderboards = new LanLeaderboards();
	m_matchMaking = new LanMatchMaking(m_discoveryManager, m_userHandle);
	m_saveData = new LanSaveData();
	m_statistics = new LanStatistics();
	m_user = new LanUser(m_discoveryManager, m_socket);
	m_voiceChat = new LanVoiceChat();

	return true;
}

void LanSessionManager::destroy()
{
	safeDestroy(m_discoveryManager);
}

bool LanSessionManager::update()
{
	if (m_matchMaking)
		m_matchMaking->update();

	if (m_user)
		m_user->update();

	return true;
}

std::wstring LanSessionManager::getLanguageCode() const
{
	return L"en";
}

bool LanSessionManager::isConnected() const
{
	return true;
}

bool LanSessionManager::requireFullScreen() const
{
	return false;
}

bool LanSessionManager::requireUserAttention() const
{
	return false;
}

bool LanSessionManager::haveDLC(const std::wstring& id) const
{
	return true;
}

bool LanSessionManager::buyDLC(const std::wstring& id) const
{
	return true;
}

bool LanSessionManager::navigateUrl(const net::Url& url) const
{
	return false;
}

uint64_t LanSessionManager::getCurrentUserHandle() const
{
	return m_userHandle;
}

bool LanSessionManager::getFriends(std::vector< uint64_t >& outFriends, bool onlineOnly) const
{
	return true;
}

bool LanSessionManager::findFriend(const std::wstring& name, uint64_t& outFriendUserHandle) const
{
	return false;
}

bool LanSessionManager::haveP2PData() const
{
	if (m_socket)
		return m_socket->select(true, false, false, 0) > 0;
	else
		return false;
}

uint32_t LanSessionManager::receiveP2PData(void* data, uint32_t size, uint64_t& outFromUserHandle) const
{
	if (!haveP2PData())
		return 0;

	net::SocketAddressIPv4 fromAddr;
	int32_t nbytes;

	if ((nbytes = m_socket->recvFrom(data, size, &fromAddr)) <= 0)
		return 0;

	RefArray< net::NetworkService > userServices;
	m_discoveryManager->findServices< net::NetworkService >(userServices);

	for (RefArray< net::NetworkService >::const_iterator i = userServices.begin(); i != userServices.end(); ++i)
	{
		if ((*i)->getType() != c_keyServiceTypeUser)
			continue;

		const PropertyGroup* propertyGroup = (*i)->getProperties();
		T_ASSERT (propertyGroup);

		uint32_t addr = propertyGroup->getProperty< int32_t >(c_keyUserAddr);
		uint16_t port = propertyGroup->getProperty< int32_t >(c_keyUserPort);

		if (fromAddr.getAddr() == addr && fromAddr.getPort() == port)
		{
			outFromUserHandle = propertyGroup->getProperty< int32_t >(c_keyUserHandle);
			return nbytes;
		}
	}

	return 0;
}

uint32_t LanSessionManager::getCurrentGameCount() const
{
	return 0;
}

IAchievementsProvider* LanSessionManager::getAchievements() const
{
	return m_achievements;
}

ILeaderboardsProvider* LanSessionManager::getLeaderboards() const
{
	return m_leaderboards;
}

IMatchMakingProvider* LanSessionManager::getMatchMaking() const
{
	return m_matchMaking;
}

ISaveDataProvider* LanSessionManager::getSaveData() const
{
	return m_saveData;
}

IStatisticsProvider* LanSessionManager::getStatistics() const
{
	return m_statistics;
}

IUserProvider* LanSessionManager::getUser() const
{
	return m_user;
}

IVideoSharingProvider* LanSessionManager::getVideoSharing() const
{
	return 0;
}

IVoiceChatProvider* LanSessionManager::getVoiceChat() const
{
	return m_voiceChat;
}

	}
}
