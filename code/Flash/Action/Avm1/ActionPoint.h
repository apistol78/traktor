#ifndef traktor_flash_ActionPoint_H
#define traktor_flash_ActionPoint_H

#include "Flash/Action/ActionObject.h"

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

/*! \brief ActionScript point.
 * \ingroup Flash
 */
class T_DLLCLASS ActionPoint : public ActionObject
{
	T_RTTI_CLASS;

public:
	avm_number_t x;
	avm_number_t y;

	ActionPoint();

	ActionPoint(avm_number_t x_, avm_number_t y_);
};

	}
}

#endif	// traktor_flash_ActionPoint_H
