#pragma once

#include <string>
#include "Core/Object.h"
#include "Core/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ONLINE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Result;

	namespace online
	{

class T_DLLCLASS IAchievements : public Object
{
	T_RTTI_CLASS;

public:
	virtual bool ready() const = 0;

	virtual bool enumerate(std::set< std::wstring >& outAchievementIds) const = 0;

	virtual bool have(const std::wstring& achievementId) const = 0;

	virtual Ref< Result > set(const std::wstring& achievementId, bool reward) = 0;
};

	}
}

