#ifndef traktor_online_LanUser_H
#define traktor_online_LanUser_H

#include <map>
#include "Online/Provider/IUserProvider.h"

namespace traktor
{
	namespace online
	{

class LanUser : public IUserProvider
{
	T_RTTI_CLASS;

public:
	LanUser();

	virtual bool getName(uint64_t userHandle, std::wstring& outName);

	virtual Ref< drawing::Image > getImage(uint64_t userHandle) const;

	virtual bool isFriend(uint64_t userHandle);

	virtual bool isMemberOfGroup(uint64_t userHandle, const std::wstring& groupName) const;

	virtual bool joinGroup(uint64_t userHandle, const std::wstring& groupName);

	virtual bool invite(uint64_t userHandle);

	virtual bool setPresenceValue(uint64_t userHandle, const std::wstring& key, const std::wstring& value);

	virtual bool getPresenceValue(uint64_t userHandle, const std::wstring& key, std::wstring& outValue);

	virtual bool isP2PAllowed(uint64_t userHandle) const;

	virtual bool isP2PRelayed(uint64_t userHandle) const;

	virtual bool sendP2PData(uint64_t userHandle, const void* data, size_t size, bool reliable);

private:
	std::map< uint64_t, std::wstring > m_nameCache;
	uint32_t m_nameIndex;
};

	}
}

#endif	// traktor_online_LanUser_H
