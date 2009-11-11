#ifndef traktor_flash_AsAccessibility_H
#define traktor_flash_AsAccessibility_H

#include "Flash/Action/ActionClass.h"

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
	T_RTTI_CLASS(AsAccessibility)

public:
	static Ref< AsAccessibility > getInstance();

private:
	AsAccessibility();

	void createPrototype();

	virtual ActionValue construct(ActionContext* context, const args_t& args);

	void Accessibility_isActive(CallArgs& ca);

	void Accessibility_updateProperties(CallArgs& ca);
};

	}
}

#endif	// traktor_flash_AsAccessibility_H
