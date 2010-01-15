#ifndef traktor_online_IAchievement_H
#define traktor_online_IAchievement_H

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

/*! \brief Game achievement.
 * \ingroup Online
 */
class T_DLLCLASS IAchievement : public Object
{
	T_RTTI_CLASS;

public:
	/*! \brief Human readable name of achievement.
	 *
	 * \return Name of achievement.
	 */
	virtual std::wstring getName() const = 0;

	/*! \brief Score rating of achievement.
	 *
	 * \return Score rating.
	 */
	virtual uint32_t getScoreRating() const = 0;
};

	}
}

#endif	// traktor_online_IAchievement_H
