#ifndef traktor_flash_As_mx_transitions_easing_Back_H
#define traktor_flash_As_mx_transitions_easing_Back_H

#include "Flash/Action/ActionObject.h"

namespace traktor
{
	namespace flash
	{

class As_mx_transitions_easing_Back : public ActionObject
{
	T_RTTI_CLASS;

public:
	As_mx_transitions_easing_Back(ActionContext* context);

private:
	void Back_easeIn(CallArgs& ca);

	void Back_easeInOut(CallArgs& ca);

	void Back_easeOut(CallArgs& ca);
};

	}
}

#endif	// traktor_flash_As_mx_transitions_easing_Back_H
