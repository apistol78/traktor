#include <ctime>
#include <limits>
#include "Core/Log/Log.h"
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

const wchar_t* c_keyServiceTypeUserInLobby = L"UIL";
const wchar_t* c_keyLobbyHandle = L"LH";
const wchar_t* c_keyLobbyOwner = L"LO";
const wchar_t* c_keyUserHandle = L"UH";
const wchar_t* c_keyOrder = L"O";
const wchar_t* c_keyPrefixLobbyMeta = L"LM";
const wchar_t* c_keyPrefixUserMeta = L"UM";

int32_t generateUniqueKey()
{
	Guid unique = Guid::create();
	const uint8_t* p = unique;
	int32_t k = *(const int32_t*)&p[0] ^ *(const int32_t*)&p[4] ^ *(const int32_t*)&p[8] ^ *(const int32_t*)&p[12];
	if (k < 0) k = -k;
	return k;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.LanMatchMaking", LanMatchMaking, IMatchMakingProvider)

LanMatchMaking::LanMatchMaking(net::DiscoveryManager* discoveryManager, int32_t userHandle)
:	m_discoveryManager(discoveryManager)
,	m_userHandle(userHandle)
,	m_lobbyHandle(0)
,	m_primaryLobbyUser(0)
{
}

void LanMatchMaking::update()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	m_lobbyUsers.resize(0);

	// Find all users which is registered to same lobby as ourself.
	if (m_userInLobbyService)
	{
		Ref< net::NetworkService > primaryUserInLobbyService;
		int32_t primaryOrder = 0;

		// Get my order value first.
		const PropertyGroup* propertyGroup = m_userInLobbyService->getProperties();
		T_ASSERT (propertyGroup);

		primaryUserInLobbyService = m_userInLobbyService;
		primaryOrder = propertyGroup->getProperty< PropertyInteger >(c_keyOrder);

		// Scan all others which are in lobby.
		RefArray< net::NetworkService > userInLobbyServices;
		m_discoveryManager->findServices< net::NetworkService >(userInLobbyServices);

		for (RefArray< net::NetworkService >::const_iterator i = userInLobbyServices.begin(); i != userInLobbyServices.end(); ++i)
		{
			if ((*i)->getType() != c_keyServiceTypeUserInLobby)
				continue;

			const PropertyGroup* propertyGroup = (*i)->getProperties();
			T_ASSERT (propertyGroup);

			if (propertyGroup->getProperty< PropertyInteger >(c_keyLobbyHandle) != m_lobbyHandle)
				continue;

			m_lobbyUsers.push_back(propertyGroup->getProperty< PropertyInteger >(c_keyUserHandle));

			if (propertyGroup->getProperty< PropertyInteger >(c_keyOrder) > primaryOrder)
			{
				primaryUserInLobbyService = *i;
				primaryOrder = propertyGroup->getProperty< PropertyInteger >(c_keyOrder);
			}
		}

		if (primaryUserInLobbyService)
		{
			const PropertyGroup* propertyGroup = primaryUserInLobbyService->getProperties();
			T_ASSERT (propertyGroup);

			m_primaryLobbyUser = propertyGroup->getProperty< PropertyInteger >(c_keyLobbyOwner);

			// If I have highest order then I must also ensure lobby owner exist; else I take responsibility.
			if (primaryUserInLobbyService == m_userInLobbyService && m_primaryLobbyUser != m_userHandle)
			{
				if (std::find(m_lobbyUsers.begin(), m_lobbyUsers.end(), m_primaryLobbyUser) == m_lobbyUsers.end())
				{
					log::info << L"Lobby owner disappeared; claiming ownership" << Endl;
					m_primaryLobbyUser = m_userHandle;
				}
			}

			if (m_primaryLobbyUser != m_userHandle)
			{
				for (RefArray< net::NetworkService >::const_iterator i = userInLobbyServices.begin(); i != userInLobbyServices.end(); ++i)
				{
					if ((*i)->getType() != c_keyServiceTypeUserInLobby)
						continue;

					const PropertyGroup* userPropertyGroup = (*i)->getProperties();
					T_ASSERT (userPropertyGroup);

					if (userPropertyGroup->getProperty< PropertyInteger >(c_keyLobbyHandle) != m_lobbyHandle)
						continue;

					if (userPropertyGroup->getProperty< PropertyInteger >(c_keyUserHandle) == m_primaryLobbyUser)
					{
						Ref< PropertyGroup > propertyGroup = DeepClone(m_userInLobbyService->getProperties()).create< PropertyGroup >();

						Ref< PropertyGroup > lobbyMetaGroup = userPropertyGroup->getProperty< PropertyGroup >(c_keyPrefixLobbyMeta);
						propertyGroup->setProperty(c_keyPrefixLobbyMeta, DeepClone(lobbyMetaGroup).create< PropertyGroup >());

						Ref< net::NetworkService > userInLobbyService = new net::NetworkService(c_keyServiceTypeUserInLobby, propertyGroup);
						m_discoveryManager->replaceService(m_userInLobbyService, userInLobbyService);
						m_userInLobbyService = userInLobbyService;

						break;
					}
				}
			}

			setLobbyOwner(m_lobbyHandle, m_primaryLobbyUser);
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
		if ((*i)->getType() != c_keyServiceTypeUserInLobby)
			continue;

		const PropertyGroup* propertyGroup = (*i)->getProperties();
		T_ASSERT (propertyGroup);

		uint64_t serviceLobbyHandle = propertyGroup->getProperty< PropertyInteger >(c_keyLobbyHandle);
		if (std::find(outLobbyHandles.begin(), outLobbyHandles.end(), serviceLobbyHandle) == outLobbyHandles.end())
			outLobbyHandles.push_back(serviceLobbyHandle);
	}

	return true;
}

bool LanMatchMaking::createLobby(uint32_t maxUsers, LobbyAccess access, uint64_t& outLobbyHandle)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	m_lobbyHandle = generateUniqueKey();

	// Create user-in-lobby service
	{
		Ref< PropertyGroup > propertyGroup = new PropertyGroup();
		propertyGroup->setProperty< PropertyInteger >(c_keyLobbyHandle, m_lobbyHandle);
		propertyGroup->setProperty< PropertyInteger >(c_keyLobbyOwner, m_userHandle);
		propertyGroup->setProperty< PropertyInteger >(c_keyUserHandle, m_userHandle);
		propertyGroup->setProperty< PropertyInteger >(c_keyOrder, std::numeric_limits< int32_t >::max());
		m_userInLobbyService = new net::NetworkService(c_keyServiceTypeUserInLobby, propertyGroup);
		m_discoveryManager->addService(m_userInLobbyService);
	}

	update();

	outLobbyHandle = m_lobbyHandle;
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

	RefArray< net::NetworkService > userInLobbyServices;
	m_discoveryManager->findServices< net::NetworkService >(userInLobbyServices);

	// Find smallest order in lobby.
	const PropertyGroup* smallestPropertyGroup = 0;
	int32_t smallestOrder = 0;

	for (RefArray< net::NetworkService >::const_iterator i = userInLobbyServices.begin(); i != userInLobbyServices.end(); ++i)
	{
		if ((*i)->getType() != c_keyServiceTypeUserInLobby)
			continue;

		const PropertyGroup* servicePropertyGroup = (*i)->getProperties();
		T_ASSERT (servicePropertyGroup);

		int32_t serviceLobbyHandle = servicePropertyGroup->getProperty< PropertyInteger >(c_keyLobbyHandle);
		if (serviceLobbyHandle == lobbyHandle)
		{
			int32_t order = servicePropertyGroup->getProperty< PropertyInteger >(c_keyOrder);
			if (!smallestPropertyGroup || order < smallestOrder)
			{
				smallestPropertyGroup = servicePropertyGroup;
				smallestOrder = order;
			}
		}
	}

	if (!smallestPropertyGroup)
		return false;

	T_ASSERT (smallestOrder > 0);

	// Create user-in-lobby service
	{
		Ref< PropertyGroup > propertyGroup = DeepClone(smallestPropertyGroup).create< PropertyGroup >();
		propertyGroup->setProperty< PropertyInteger >(c_keyLobbyHandle, lobbyHandle);
		propertyGroup->setProperty< PropertyInteger >(c_keyUserHandle, m_userHandle);
		propertyGroup->setProperty< PropertyInteger >(c_keyOrder, smallestOrder - 1);
		m_userInLobbyService = new net::NetworkService(c_keyServiceTypeUserInLobby, propertyGroup);
		m_discoveryManager->addService(m_userInLobbyService);
	}

	m_lobbyHandle = lobbyHandle;

	update();
	return true;
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
	propertyGroup->setProperty< PropertyString >(std::wstring(c_keyPrefixLobbyMeta) + L"/" + key, value);

	Ref< net::NetworkService > userInLobbyService = new net::NetworkService(c_keyServiceTypeUserInLobby, propertyGroup);
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

		outValue = propertyGroup->getProperty< PropertyString >(std::wstring(c_keyPrefixLobbyMeta) + L"/" + key);
		return true;
	}
	else
	{
		RefArray< net::NetworkService > userInLobbyServices;
		m_discoveryManager->findServices< net::NetworkService >(userInLobbyServices);

		for (RefArray< net::NetworkService >::const_iterator i = userInLobbyServices.begin(); i != userInLobbyServices.end(); ++i)
		{
			if ((*i)->getType() != c_keyServiceTypeUserInLobby)
				continue;

			const PropertyGroup* propertyGroup = (*i)->getProperties();
			T_ASSERT (propertyGroup);

			uint64_t serviceLobbyHandle = propertyGroup->getProperty< PropertyInteger >(c_keyLobbyHandle);
			if (serviceLobbyHandle != lobbyHandle)
				continue;

			outValue = propertyGroup->getProperty< PropertyString >(std::wstring(c_keyPrefixLobbyMeta) + L"/" + key);
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
	propertyGroup->setProperty< PropertyString >(std::wstring(c_keyPrefixUserMeta) + L"/" + key, value);

	Ref< net::NetworkService > userInLobbyService = new net::NetworkService(c_keyServiceTypeUserInLobby, propertyGroup);
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
	outUserHandles.resize(m_lobbyUsers.size());
	for (size_t i = 0; i < m_lobbyUsers.size(); ++i)
		outUserHandles[i] = m_lobbyUsers[i];
	std::sort(outUserHandles.begin(), outUserHandles.end());
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
		if ((*i)->getType() != c_keyServiceTypeUserInLobby)
			continue;

		const PropertyGroup* propertyGroup = (*i)->getProperties();
		T_ASSERT (propertyGroup);

		uint64_t serviceLobbyHandle = propertyGroup->getProperty< PropertyInteger >(c_keyLobbyHandle);
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
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	if (!m_userInLobbyService)
		return false;

	const PropertyGroup* currentPropertyGroup = m_userInLobbyService->getProperties();
	T_ASSERT (currentPropertyGroup);

	if (currentPropertyGroup->getProperty< PropertyInteger >(c_keyLobbyOwner) != int32_t(userHandle))
	{
		Ref< PropertyGroup > propertyGroup = DeepClone(currentPropertyGroup).create< PropertyGroup >();
		propertyGroup->setProperty< PropertyInteger >(c_keyLobbyOwner, int32_t(userHandle));

		Ref< net::NetworkService > userInLobbyService = new net::NetworkService(c_keyServiceTypeUserInLobby, propertyGroup);
		m_discoveryManager->replaceService(m_userInLobbyService, userInLobbyService);
		m_userInLobbyService = userInLobbyService;
	}

	return true;
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
