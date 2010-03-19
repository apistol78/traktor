#ifndef traktor_flash_As_mx_transitions_easing_Strong_H
#define traktor_flash_As_mx_transitions_easing_Strong_H

#include "Flash/Action/Avm1/ActionClass.h"

namespace traktor
{
	namespace flash
	{

class As_mx_transitions_easing_Strong : public ActionClass
{
	T_RTTI_CLASS;

public:
	static Ref< As_mx_transitions_easing_Strong > getInstance();

private:
	As_mx_transitions_easing_Strong();

	void createPrototype();

	virtual ActionValue construct(ActionContext* context, const args_t& args);

	void Strong_easeIn(CallArgs& ca);

	void Strong_easeInOut(CallArgs& ca);

	void Strong_easeOut(CallArgs& ca);
};

	}
}

#endif	// traktor_flash_As_mx_transitions_easing_Strong_H
