#ifndef traktor_flash_ActionBoolean_H
#define traktor_flash_ActionBoolean_H

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

/*! \brief ActionScript boolean wrapper.
 * \ingroup Flash
 */
class T_DLLCLASS ActionBoolean : public ActionObject
{
	T_RTTI_CLASS(ActionBoolean)

public:
	ActionBoolean(bool value);

	bool get() const { return m_value; }

private:
	bool m_value;
};

	}
}

#endif	// traktor_flash_ActionBoolean_H
