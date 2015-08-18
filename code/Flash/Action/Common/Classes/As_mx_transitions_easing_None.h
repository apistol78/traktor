#ifndef traktor_flash_As_mx_transitions_easing_None_H
#define traktor_flash_As_mx_transitions_easing_None_H

#include "Flash/Action/ActionObject.h"

namespace traktor
{
	namespace flash
	{

class As_mx_transitions_easing_None : public ActionObject
{
	T_RTTI_CLASS;

public:
	As_mx_transitions_easing_None(ActionContext* context);

private:
	void None_easeIn(CallArgs& ca);

	void None_easeInOut(CallArgs& ca);

	void None_easeOut(CallArgs& ca);

	void None_easeNone(CallArgs& ca);
};

	}
}

#endif	// traktor_flash_As_mx_transitions_easing_None_H
