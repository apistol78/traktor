#ifndef traktor_flash_As_mx_transitions_easing_None_H
#define traktor_flash_As_mx_transitions_easing_None_H

#include "Flash/Action/Avm1/ActionClass.h"

namespace traktor
{
	namespace flash
	{

class As_mx_transitions_easing_None : public ActionClass
{
	T_RTTI_CLASS;

public:
	As_mx_transitions_easing_None(ActionContext* context);

	virtual void init(ActionObject* self, const ActionValueArray& args) const;

	virtual void coerce(ActionObject* self) const;

private:
	void None_easeIn(CallArgs& ca);

	void None_easeInOut(CallArgs& ca);

	void None_easeOut(CallArgs& ca);

	void None_easeNone(CallArgs& ca);
};

	}
}

#endif	// traktor_flash_As_mx_transitions_easing_None_H
