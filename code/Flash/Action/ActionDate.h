#ifndef traktor_flash_ActionDate_H
#define traktor_flash_ActionDate_H

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

/*! \brief ActionScript date.
 * \ingroup Flash
 */
class T_DLLCLASS ActionDate : public ActionObject
{
	T_RTTI_CLASS(ActionDate)

public:
	ActionDate();
};

	}
}

#endif	// traktor_flash_ActionDate_H
