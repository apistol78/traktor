#ifndef traktor_flash_Number_H
#define traktor_flash_Number_H

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

/*! \brief ActionScript number wrapper.
 * \ingroup Flash
 */
class T_DLLCLASS Number : public ActionObject
{
	T_RTTI_CLASS;

public:
	Number(avm_number_t value);

	avm_number_t get() const { return m_value; }

private:
	avm_number_t m_value;
};

	}
}

#endif	// traktor_flash_Number_H
