#ifndef traktor_flash_As_mx_transitions_easing_Back_H
#define traktor_flash_As_mx_transitions_easing_Back_H

#include "Flash/Action/Avm1/ActionClass.h"

namespace traktor
{
	namespace flash
	{

class As_mx_transitions_easing_Back : public ActionClass
{
	T_RTTI_CLASS;

public:
	As_mx_transitions_easing_Back();

	virtual Ref< ActionObject > alloc(ActionContext* context);

	virtual void init(ActionContext* context, ActionObject* self, const ActionValueArray& args);

private:
	void Back_easeIn(CallArgs& ca);

	void Back_easeInOut(CallArgs& ca);

	void Back_easeOut(CallArgs& ca);
};

	}
}

#endif	// traktor_flash_As_mx_transitions_easing_Back_H
