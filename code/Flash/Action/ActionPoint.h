#ifndef traktor_flash_ActionPoint_H
#define traktor_flash_ActionPoint_H

#include "Flash/Action/ActionObject.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

/*! \brief ActionScript point.
 * \ingroup Flash
 */
class T_DLLCLASS ActionPoint : public ActionObject
{
	T_RTTI_CLASS;

public:
	double x;
	double y;

	ActionPoint();

	ActionPoint(double x_, double y_);
};

	}
}

#endif	// traktor_flash_ActionPoint_H
