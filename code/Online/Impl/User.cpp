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

Ref< drawing::Image > User::getImage() const
{
	return m_userProvider->getImage(m_handle);
}

uint64_t User::getGlobalId() const
{
	return m_handle;
}

bool User::isFriend() const
{
	return m_userProvider->isFriend(m_handle);
}

bool User::invite()
{
	return m_userProvider->invite(m_handle);
}

bool User::setPresenceValue(const std::wstring& key, const std::wstring& value)
{
	return m_userProvider->setPresenceValue(m_handle, key, value);
}

bool User::getPresenceValue(const std::wstring& key, std::wstring& outValue) const
{
	return m_userProvider->getPresenceValue(m_handle, key, outValue);
}

bool User::isP2PAllowed() const
{
	return m_userProvider->isP2PAllowed(m_handle);
}

bool User::isP2PRelayed() const
{
	return m_userProvider->isP2PRelayed(m_handle);
}

bool User::sendP2PData(const void* data, size_t size)
{
	return m_userProvider->sendP2PData(m_handle, data, size);
}

User::User(IUserProvider* userProvider, uint64_t handle)
:	m_userProvider(userProvider)
,	m_handle(handle)
{
}

	}
}
