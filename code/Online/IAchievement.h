#ifndef traktor_online_IAchievement_H
#define traktor_online_IAchievement_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ONLINE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif 

namespace traktor
{
	namespace online
	{

class T_DLLCLASS IAchievement : public Object
{
	T_RTTI_CLASS(IAchievement)

public:
	virtual std::wstring getName() const = 0;

	virtual uint32_t getScoreRating() const = 0;
};

	}
}

#endif	// traktor_online_IAchievement_H
