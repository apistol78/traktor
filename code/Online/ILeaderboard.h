#ifndef traktor_online_ILeaderboard_H
#define traktor_online_ILeaderboard_H

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

/*! \brief Leaderboard interface.
 * \ingroup Online
 */
class T_DLLCLASS ILeaderboard : public Object
{
	T_RTTI_CLASS;

public:
	virtual uint32_t getRank() const = 0;

	virtual bool setScore(int32_t score) = 0;

	virtual int32_t getScore() const = 0;
};

	}
}

#endif	// traktor_online_ILeaderboard_H
