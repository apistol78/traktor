#ifndef traktor_flash_AsNumber_H
#define traktor_flash_AsNumber_H

#include "Flash/Action/Avm1/ActionClass.h"

namespace traktor
{
	namespace flash
	{

class Number;

/*! \brief Number class.
 * \ingroup Flash
 */
class AsNumber : public ActionClass
{
	T_RTTI_CLASS;

public:
	AsNumber(ActionContext* context);

	virtual void init(ActionObject* self, const ActionValueArray& args);

	virtual void coerce(ActionObject* self) const;

private:
	std::wstring Number_toString(const Number* self) const;

	avm_number_t Number_valueOf(const Number* self) const;
};

	}
}

#endif	// traktor_flash_AsNumber_H
