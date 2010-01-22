#ifndef traktor_flash_ActionNumber_H
#define traktor_flash_ActionNumber_H

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
class T_DLLCLASS ActionNumber : public ActionObject
{
	T_RTTI_CLASS;

public:
	ActionNumber(avm_number_t value);

	avm_number_t get() const { return m_value; }

private:
	avm_number_t m_value;
};

	}
}

#endif	// traktor_flash_ActionNumber_H
