#ifndef traktor_flash_As_flash_display_InteractiveObject_H
#define traktor_flash_As_flash_display_InteractiveObject_H

#include "Flash/Action/ActionClass.h"

namespace traktor
{
	namespace flash
	{

class As_flash_display_InteractiveObject : public ActionClass
{
	T_RTTI_CLASS;

public:
	As_flash_display_InteractiveObject(ActionContext* context);

	virtual void initialize(ActionObject* self) T_OVERRIDE T_FINAL;

	virtual void construct(ActionObject* self, const ActionValueArray& args) T_OVERRIDE T_FINAL;

	virtual ActionValue xplicit(const ActionValueArray& args) T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_flash_As_flash_display_InteractiveObject_H
