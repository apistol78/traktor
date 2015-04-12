#include "Core/Misc/String.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Net/SocketAddressIPv4.h"
#include "Net/UdpSocket.h"
#include "Net/Discovery/DiscoveryManager.h"
#include "Net/Discovery/NetworkService.h"
#include "Online/Lan/LanUser.h"

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

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.LocalUser", LanUser, IUserProvider)

LanUser::LanUser(net::DiscoveryManager* discoveryManager, net::UdpSocket* socket)
:	m_discoveryManager(discoveryManager)
,	m_socket(socket)
{
}

bool LanUser::getName(uint64_t userHandle, std::wstring& outName)
{
	outName = std::wstring(L"User_") + toString(int32_t(userHandle));
	return true;
}

Ref< drawing::Image > LanUser::getImage(uint64_t userHandle) const
{
	return 0;
}

bool LanUser::isFriend(uint64_t userHandle)
{
	return false;
}

bool LanUser::isMemberOfGroup(uint64_t userHandle, const std::wstring& groupName) const
{
	return false;
}

bool LanUser::joinGroup(uint64_t userHandle, const std::wstring& groupName)
{
	return false;
}

bool LanUser::invite(uint64_t userHandle)
{
	return false;
}

bool LanUser::setPresenceValue(uint64_t userHandle, const std::wstring& key, const std::wstring& value)
{
	return false;
}

bool LanUser::getPresenceValue(uint64_t userHandle, const std::wstring& key, std::wstring& outValue)
{
	return false;
}

bool LanUser::isP2PAllowed(uint64_t userHandle) const
{
	return true;
}

bool LanUser::isP2PRelayed(uint64_t userHandle) const
{
	return false;
}

bool LanUser::sendP2PData(uint64_t userHandle, const void* data, size_t size, bool reliable)
{
	RefArray< net::NetworkService > userServices;
	m_discoveryManager->findServices< net::NetworkService >(userServices);

	for (RefArray< net::NetworkService >::const_iterator i = userServices.begin(); i != userServices.end(); ++i)
	{
		if ((*i)->getType() != c_keyServiceTypeUser)
			continue;

		const PropertyGroup* propertyGroup = (*i)->getProperties();
		T_ASSERT (propertyGroup);

		if (propertyGroup->getProperty< PropertyInteger >(c_keyUserHandle) != userHandle)
			continue;

		uint32_t addr = propertyGroup->getProperty< PropertyInteger >(c_keyUserAddr);
		uint16_t port = propertyGroup->getProperty< PropertyInteger >(c_keyUserPort);

		int32_t result = m_socket->sendTo(
			net::SocketAddressIPv4(addr, port),
			data,
			int(size)
		);
		return bool(result == int32_t(size));
	}

	return false;
}

	}
}
