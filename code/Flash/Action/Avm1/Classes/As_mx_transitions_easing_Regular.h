#ifndef traktor_flash_As_mx_transitions_easing_Regular_H
#define traktor_flash_As_mx_transitions_easing_Regular_H

#include "Flash/Action/ActionObject.h"

namespace traktor
{
	namespace flash
	{

class As_mx_transitions_easing_Regular : public ActionObject
{
	T_RTTI_CLASS;

public:
	As_mx_transitions_easing_Regular(ActionContext* context);

private:
	void Regular_easeIn(CallArgs& ca);

	void Regular_easeInOut(CallArgs& ca);

	void Regular_easeOut(CallArgs& ca);
};

	}
}

#endif	// traktor_flash_As_mx_transitions_easing_Regular_H
