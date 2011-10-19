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
	As_mx_transitions_easing_None();

	virtual Ref< ActionObject > alloc(ActionContext* context);

	virtual void init(ActionContext* context, ActionObject* self, const ActionValueArray& args);

private:
	void None_easeIn(CallArgs& ca);

	void None_easeInOut(CallArgs& ca);

	void None_easeOut(CallArgs& ca);

	void None_easeNone(CallArgs& ca);
};

	}
}

#endif	// traktor_flash_As_mx_transitions_easing_None_H
