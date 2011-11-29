#ifndef traktor_online_ILobby_H
#define traktor_online_ILobby_H

#include "Online/UserArrayResult.h"

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

class T_DLLCLASS ILobby : public Object
{
	T_RTTI_CLASS;

public:
	virtual Ref< Result > setMetaValue(const std::wstring& key, const std::wstring& value) = 0;

	virtual bool getMetaValue(const std::wstring& key, std::wstring& outValue) const = 0;

	virtual Ref< Result > join() = 0;

	virtual Ref< Result > leave() = 0;

	virtual Ref< UserArrayResult > getParticipants() = 0;
};

	}
}

#endif	// traktor_online_ILobby_H
