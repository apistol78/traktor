#ifndef traktor_flash_Date_H
#define traktor_flash_Date_H

#include "Flash/Action/ActionObjectRelay.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

/*! \brief ActionScript date.
 * \ingroup Flash
 */
class T_DLLCLASS Date : public ActionObjectRelay
{
	T_RTTI_CLASS;

public:
	Date();
};

	}
}

#endif	// traktor_flash_Date_H
