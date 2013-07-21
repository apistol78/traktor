#ifndef traktor_online_IUser_H
#define traktor_online_IUser_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ONLINE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace drawing
	{

class Image;

	}

	namespace online
	{

class T_DLLCLASS IUser : public Object
{
	T_RTTI_CLASS;

public:
	virtual bool getName(std::wstring& outName) const = 0;

	virtual Ref< drawing::Image > getImage() const = 0;

	virtual uint64_t getGlobalId() const = 0;

	virtual bool isFriend() const = 0;

	virtual bool invite() = 0;

	virtual bool setPresenceValue(const std::wstring& key, const std::wstring& value) = 0;

	virtual bool getPresenceValue(const std::wstring& key, std::wstring& outValue) const = 0;

	virtual bool isP2PAllowed() const = 0;

	virtual bool isP2PRelayed() const = 0;

	virtual bool sendP2PData(const void* data, size_t size) = 0;
};

	}
}

#endif	// traktor_online_IUser_H
