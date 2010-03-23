#ifndef traktor_flash_Point_H
#define traktor_flash_Point_H

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
class T_DLLCLASS Point : public ActionObject
{
	T_RTTI_CLASS;

public:
	avm_number_t x;
	avm_number_t y;

	Point();

	Point(avm_number_t x_, avm_number_t y_);

	virtual std::wstring toString() const;
};

	}
}

#endif	// traktor_flash_Point_H
