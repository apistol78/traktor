#ifndef traktor_flash_AsAccessibility_H
#define traktor_flash_AsAccessibility_H

#include "Flash/Action/Avm1/ActionClass.h"

namespace traktor
{
	namespace flash
	{

struct CallArgs;

/*! \brief Accessibility class.
 * \ingroup Flash
 */
class AsAccessibility : public ActionClass
{
	T_RTTI_CLASS;

public:
	AsAccessibility(ActionContext* context);

	virtual void init(ActionObject* self, const ActionValueArray& args);

	virtual void coerce(ActionObject* self) const;

private:
	void Accessibility_isActive(CallArgs& ca);

	void Accessibility_updateProperties(CallArgs& ca);
};

	}
}

#endif	// traktor_flash_AsAccessibility_H
