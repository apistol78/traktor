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
	static Ref< As_mx_transitions_easing_Bounce > getInstance();

private:
	As_mx_transitions_easing_Bounce();

	void createPrototype();

	virtual ActionValue construct(ActionContext* context, const args_t& args);

	void Bounce_easeIn(CallArgs& ca);

	void Bounce_easeInOut(CallArgs& ca);

	void Bounce_easeOut(CallArgs& ca);
};

	}
}

#endif	// traktor_flash_As_mx_transitions_easing_Bounce_H
