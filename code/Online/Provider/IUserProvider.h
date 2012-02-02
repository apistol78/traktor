#ifndef traktor_online_IUserProvider_H
#define traktor_online_IUserProvider_H

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
	namespace online
	{

class T_DLLCLASS IUserProvider : public Object
{
	T_RTTI_CLASS;

public:
	virtual bool getName(uint64_t userHandle, std::wstring& outName) = 0;

	virtual bool setMetaValue(uint64_t userHandle, const std::wstring& key, const std::wstring& value) = 0;

	virtual bool getMetaValue(uint64_t userHandle, const std::wstring& key, std::wstring& outValue) = 0;

	virtual bool sendP2PData(uint64_t userHandle, const void* data, size_t size, bool reliable) = 0;
};

	}
}

#endif	// traktor_online_IUserProvider_H
