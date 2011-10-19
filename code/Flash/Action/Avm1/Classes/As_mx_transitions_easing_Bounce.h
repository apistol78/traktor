#ifndef traktor_flash_As_mx_transitions_easing_Bounce_H
#define traktor_flash_As_mx_transitions_easing_Bounce_H

#include "Flash/Action/Avm1/ActionClass.h"

namespace traktor
{
	namespace flash
	{

class As_mx_transitions_easing_Bounce : public ActionClass
{
	T_RTTI_CLASS;

public:
	As_mx_transitions_easing_Bounce();

	virtual Ref< ActionObject > alloc(ActionContext* context);

	virtual void init(ActionContext* context, ActionObject* self, const ActionValueArray& args);

private:
	void Bounce_easeIn(CallArgs& ca);

	void Bounce_easeInOut(CallArgs& ca);

	void Bounce_easeOut(CallArgs& ca);
};

	}
}

#endif	// traktor_flash_As_mx_transitions_easing_Bounce_H
