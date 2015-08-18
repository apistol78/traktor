#ifndef traktor_flash_As_mx_transitions_easing_Elastic_H
#define traktor_flash_As_mx_transitions_easing_Elastic_H

#include "Flash/Action/ActionObject.h"

namespace traktor
{
	namespace flash
	{

class As_mx_transitions_easing_Elastic : public ActionObject
{
	T_RTTI_CLASS;

public:
	As_mx_transitions_easing_Elastic(ActionContext* context);

private:
	void Elastic_easeIn(CallArgs& ca);

	void Elastic_easeInOut(CallArgs& ca);

	void Elastic_easeOut(CallArgs& ca);
};

	}
}

#endif	// traktor_flash_As_mx_transitions_easing_Elastic_H
