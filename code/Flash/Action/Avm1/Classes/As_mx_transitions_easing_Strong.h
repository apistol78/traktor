#ifndef traktor_flash_As_mx_transitions_easing_Strong_H
#define traktor_flash_As_mx_transitions_easing_Strong_H

#include "Flash/Action/ActionObject.h"

namespace traktor
{
	namespace flash
	{

class As_mx_transitions_easing_Strong : public ActionObject
{
	T_RTTI_CLASS;

public:
	As_mx_transitions_easing_Strong(ActionContext* context);

private:
	void Strong_easeIn(CallArgs& ca);

	void Strong_easeInOut(CallArgs& ca);

	void Strong_easeOut(CallArgs& ca);
};

	}
}

#endif	// traktor_flash_As_mx_transitions_easing_Strong_H
