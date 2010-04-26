#ifndef traktor_flash_As_mx_transitions_easing_Regular_H
#define traktor_flash_As_mx_transitions_easing_Regular_H

#include "Flash/Action/Avm1/ActionClass.h"

namespace traktor
{
	namespace flash
	{

class As_mx_transitions_easing_Regular : public ActionClass
{
	T_RTTI_CLASS;

public:
	static Ref< As_mx_transitions_easing_Regular > getInstance();

private:
	As_mx_transitions_easing_Regular();

	void createPrototype();

	virtual ActionValue construct(ActionContext* context, const ActionValueArray& args);

	void Regular_easeIn(CallArgs& ca);

	void Regular_easeInOut(CallArgs& ca);

	void Regular_easeOut(CallArgs& ca);
};

	}
}

#endif	// traktor_flash_As_mx_transitions_easing_Regular_H
