#include "Online/Impl/User.h"
#include "Online/Provider/IUserProvider.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.User", User, IUser)

bool User::getName(std::wstring& outName) const
{
	return m_userProvider->getName(m_handle, outName);
}

bool User::setPresenceValue(const std::wstring& key, const std::wstring& value)
{
	return m_userProvider->setPresenceValue(m_handle, key, value);
}

bool User::getPresenceValue(const std::wstring& key, std::wstring& outValue) const
{
	return m_userProvider->getPresenceValue(m_handle, key, outValue);
}

bool User::sendP2PData(const void* data, size_t size, bool reliable)
{
	return m_userProvider->sendP2PData(m_handle, data, size, reliable);
}

User::User(IUserProvider* userProvider, uint64_t handle)
:	m_userProvider(userProvider)
,	m_handle(handle)
{
}

	}
}
