#pragma optimize( "", off )

#include <ctime>
#include "Core/Math/Random.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Thread/Acquire.h"
#include "Net/Discovery/DiscoveryManager.h"
#include "Net/Discovery/NetworkService.h"
#include "Online/Lan/LanMatchMaking.h"

namespace traktor
{
	namespace online
	{
		namespace
		{

Random s_random;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.LanMatchMaking", LanMatchMaking, IMatchMakingProvider)

LanMatchMaking::LanMatchMaking(net::DiscoveryManager* discoveryManager, uint64_t userHandle)
:	m_discoveryManager(discoveryManager)
,	m_userHandle(userHandle)
,	m_lobbyHandle(0)
,	m_primaryLobbyUser(0)
{
	s_random = Random(std::clock());
}

void LanMatchMaking::update()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	m_lobbyUsers.resize(0);

	// Find all users which is registered to same lobby as ourself.
	if (m_userInLobbyService)
	{
		Ref< net::NetworkService > primaryUserInLobbyService;
		int32_t primaryRandom = 0;

		// Add myself to list of users in lobby.
		m_lobbyUsers.push_back(m_userHandle);

		const PropertyGroup* propertyGroup = m_userInLobbyService->getProperties();
		T_ASSERT (propertyGroup);

		if (propertyGroup->getProperty< PropertyInteger >(L"R") > primaryRandom)
		{
			primaryUserInLobbyService = m_userInLobbyService;
			primaryRandom = propertyGroup->getProperty< PropertyInteger >(L"R");
		}

		// Scan all others which are in lobby.
		RefArray< net::NetworkService > userInLobbyServices;
		m_discoveryManager->findServices< net::NetworkService >(userInLobbyServices);

		for (RefArray< net::NetworkService >::const_iterator i = userInLobbyServices.begin(); i != userInLobbyServices.end(); ++i)
		{
			if ((*i)->getType() != L"UIL")
				continue;

			const PropertyGroup* propertyGroup = (*i)->getProperties();
			T_ASSERT (propertyGroup);

			if (propertyGroup->getProperty< PropertyInteger >(L"LH") != m_lobbyHandle)
				continue;

			m_lobbyUsers.push_back(propertyGroup->getProperty< PropertyInteger >(L"UH"));

			if (propertyGroup->getProperty< PropertyInteger >(L"R") > primaryRandom)
			{
				primaryUserInLobbyService = *i;
				primaryRandom = propertyGroup->getProperty< PropertyInteger >(L"R");
			}
		}

		if (primaryUserInLobbyService)
		{
			// Migrate lobby meta from primary into our own service.

			const PropertyGroup* propertyGroup = primaryUserInLobbyService->getProperties();
			T_ASSERT (propertyGroup);

			m_primaryLobbyUser = propertyGroup->getProperty< PropertyInteger >(L"UH");
		}
	}
}

bool LanMatchMaking::findMatchingLobbies(const LobbyFilter* filter, std::vector< uint64_t >& outLobbyHandles)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	RefArray< net::NetworkService > userInLobbyServices;
	m_discoveryManager->findServices< net::NetworkService >(userInLobbyServices);

	for (RefArray< net::NetworkService >::const_iterator i = userInLobbyServices.begin(); i != userInLobbyServices.end(); ++i)
	{
		if ((*i)->getType() != L"UIL")
			continue;

		const PropertyGroup* propertyGroup = (*i)->getProperties();
		T_ASSERT (propertyGroup);

		uint64_t serviceLobbyHandle = propertyGroup->getProperty< PropertyInteger >(L"LH");
		if (std::find(outLobbyHandles.begin(), outLobbyHandles.end(), serviceLobbyHandle) == outLobbyHandles.end())
			outLobbyHandles.push_back(serviceLobbyHandle);
	}

	return true;
}

bool LanMatchMaking::createLobby(uint32_t maxUsers, LobbyAccess access, uint64_t& outLobbyHandle)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	Guid unique = Guid::create();
	const uint8_t* p = unique;
	outLobbyHandle = *(const uint64_t*)&p[0] ^ *(const uint64_t*)&p[8];

	// Create user-in-lobby service
	{
		Ref< PropertyGroup > propertyGroup = new PropertyGroup();
		propertyGroup->setProperty< PropertyInteger >(L"LH", outLobbyHandle);
		propertyGroup->setProperty< PropertyInteger >(L"UH", m_userHandle);
		propertyGroup->setProperty< PropertyInteger >(L"R", s_random.next());
		m_userInLobbyService = new net::NetworkService(L"UIL", propertyGroup);
		m_discoveryManager->addService(m_userInLobbyService);
	}

	m_lobbyHandle = outLobbyHandle;
	m_primaryLobbyUser = m_userHandle;
	return true;
}

bool LanMatchMaking::acceptLobby(uint64_t& outLobbyHandle)
{
	return false;
}

bool LanMatchMaking::joinLobby(uint64_t lobbyHandle)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	leaveLobby(lobbyHandle);

	RefArray< net::NetworkService > lobbyServices;
	m_discoveryManager->findServices< net::NetworkService >(lobbyServices);
	for (RefArray< net::NetworkService >::const_iterator i = lobbyServices.begin(); i != lobbyServices.end(); ++i)
	{
		if ((*i)->getType() != L"UIL")
			continue;

		const PropertyGroup* servicePropertyGroup = (*i)->getProperties();

		int32_t serviceLobbyHandle = servicePropertyGroup->getProperty< PropertyInteger >(L"LH");
		if (serviceLobbyHandle == lobbyHandle)
		{

			// Create user-in-lobby service
			{
				Ref< PropertyGroup > propertyGroup = DeepClone(servicePropertyGroup).create< PropertyGroup >();
				propertyGroup->setProperty< PropertyInteger >(L"LH", lobbyHandle);
				propertyGroup->setProperty< PropertyInteger >(L"UH", m_userHandle);
				propertyGroup->setProperty< PropertyInteger >(L"R", s_random.next());
				m_userInLobbyService = new net::NetworkService(L"UIL", propertyGroup);
				m_discoveryManager->addService(m_userInLobbyService);
			}

			m_lobbyHandle = lobbyHandle;
			return true;
		}
	}

	return false;
}

bool LanMatchMaking::leaveLobby(uint64_t lobbyHandle)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	if (!m_userInLobbyService)
		return false;

	m_discoveryManager->removeService(m_userInLobbyService);
	m_userInLobbyService = 0;
	m_lobbyUsers.resize(0);

	return true;
}

bool LanMatchMaking::setLobbyMetaValue(uint64_t lobbyHandle, const std::wstring& key, const std::wstring& value)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	if (!m_userInLobbyService || m_primaryLobbyUser != m_userHandle)
		return false;

	Ref< PropertyGroup > propertyGroup = DeepClone(m_userInLobbyService->getProperties()).create< PropertyGroup >();
	propertyGroup->setProperty< PropertyString >(L"LM." + key, value);

	Ref< net::NetworkService > userInLobbyService = new net::NetworkService(L"UIL", propertyGroup);
	m_discoveryManager->replaceService(m_userInLobbyService, userInLobbyService);
	m_userInLobbyService = userInLobbyService;

	return true;
}

bool LanMatchMaking::getLobbyMetaValue(uint64_t lobbyHandle, const std::wstring& key, std::wstring& outValue)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	if (m_userInLobbyService && lobbyHandle == m_lobbyHandle)
	{
		const PropertyGroup* propertyGroup = m_userInLobbyService->getProperties();
		T_ASSERT (propertyGroup);

		outValue = propertyGroup->getProperty< PropertyString >(L"LM." + key);
		return true;
	}
	else
	{
		RefArray< net::NetworkService > userInLobbyServices;
		m_discoveryManager->findServices< net::NetworkService >(userInLobbyServices);

		for (RefArray< net::NetworkService >::const_iterator i = userInLobbyServices.begin(); i != userInLobbyServices.end(); ++i)
		{
			if ((*i)->getType() != L"UIL")
				continue;

			const PropertyGroup* propertyGroup = (*i)->getProperties();
			T_ASSERT (propertyGroup);

			uint64_t serviceLobbyHandle = propertyGroup->getProperty< PropertyInteger >(L"LH");
			if (serviceLobbyHandle != lobbyHandle)
				continue;

			outValue = propertyGroup->getProperty< PropertyString >(L"LM." + key);
			return true;
		}
	}

	return false;
}

bool LanMatchMaking::setLobbyParticipantMetaValue(uint64_t lobbyHandle, const std::wstring& key, const std::wstring& value)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	if (!m_userInLobbyService)
		return false;

	Ref< PropertyGroup > propertyGroup = DeepClone(m_userInLobbyService->getProperties()).create< PropertyGroup >();
	propertyGroup->setProperty< PropertyString >(L"UM." + key, value);

	Ref< net::NetworkService > userInLobbyService = new net::NetworkService(L"UIL", propertyGroup);
	m_discoveryManager->replaceService(m_userInLobbyService, userInLobbyService);
	m_userInLobbyService = userInLobbyService;

	return true;
}

bool LanMatchMaking::getLobbyParticipantMetaValue(uint64_t lobbyHandle, uint64_t userHandle, const std::wstring& key, std::wstring& outValue)
{
	return false;
}

bool LanMatchMaking::getLobbyParticipants(uint64_t lobbyHandle, std::vector< uint64_t >& outUserHandles)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	outUserHandles = m_lobbyUsers;
	return true;
}

bool LanMatchMaking::getLobbyParticipantCount(uint64_t lobbyHandle, uint32_t& outCount) const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	outCount = 0;

	RefArray< net::NetworkService > userInLobbyServices;
	m_discoveryManager->findServices< net::NetworkService >(userInLobbyServices);

	for (RefArray< net::NetworkService >::const_iterator i = userInLobbyServices.begin(); i != userInLobbyServices.end(); ++i)
	{
		if ((*i)->getType() != L"UIL")
			continue;

		const PropertyGroup* propertyGroup = (*i)->getProperties();
		T_ASSERT (propertyGroup);

		uint64_t serviceLobbyHandle = propertyGroup->getProperty< PropertyInteger >(L"LH");
		if (serviceLobbyHandle == lobbyHandle)
			++outCount;
	}

	return true;
}

bool LanMatchMaking::getLobbyMaxParticipantCount(uint64_t lobbyHandle, uint32_t& outCount) const
{
	outCount = 64;
	return true;
}

bool LanMatchMaking::getLobbyFriendsCount(uint64_t lobbyHandle, uint32_t& outCount) const
{
	outCount = 0;
	return true;
}

bool LanMatchMaking::inviteToLobby(uint64_t lobbyHandle, uint64_t userHandle)
{
	return false;
}

bool LanMatchMaking::setLobbyOwner(uint64_t lobbyHandle, uint64_t userHandle) const
{
	return false;
}

bool LanMatchMaking::getLobbyOwner(uint64_t lobbyHandle, uint64_t& outUserHandle) const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	outUserHandle = m_primaryLobbyUser;
	return true;
}

bool LanMatchMaking::createParty(uint64_t& outPartyHandle)
{
	return false;
}

bool LanMatchMaking::acceptParty(uint64_t& outPartyHandle)
{
	return false;
}

bool LanMatchMaking::joinParty(uint64_t partyHandle)
{
	return false;
}

bool LanMatchMaking::leaveParty(uint64_t partyHandle)
{
	return false;
}

bool LanMatchMaking::setPartyMetaValue(uint64_t partyHandle, const std::wstring& key, const std::wstring& value)
{
	return false;
}

bool LanMatchMaking::getPartyMetaValue(uint64_t partyHandle, const std::wstring& key, std::wstring& outValue)
{
	return false;
}

bool LanMatchMaking::setPartyParticipantMetaValue(uint64_t partyHandle, const std::wstring& key, const std::wstring& value)
{
	return false;
}

bool LanMatchMaking::getPartyParticipantMetaValue(uint64_t partyHandle, uint64_t userHandle, const std::wstring& key, std::wstring& outValue)
{
	return false;
}

bool LanMatchMaking::getPartyParticipants(uint64_t partyHandle, std::vector< uint64_t >& outUserHandles)
{
	return false;
}

bool LanMatchMaking::getPartyParticipantCount(uint64_t partyHandle, uint32_t& outCount) const
{
	return false;
}

bool LanMatchMaking::inviteToParty(uint64_t partyHandle, uint64_t userHandle)
{
	return false;
}

	}
}
