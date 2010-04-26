#ifndef traktor_flash_As_mx_transitions_easing_Elastic_H
#define traktor_flash_As_mx_transitions_easing_Elastic_H

#include "Flash/Action/Avm1/ActionClass.h"

namespace traktor
{
	namespace flash
	{

class As_mx_transitions_easing_Elastic : public ActionClass
{
	T_RTTI_CLASS;

public:
	static Ref< As_mx_transitions_easing_Elastic > getInstance();

private:
	As_mx_transitions_easing_Elastic();

	void createPrototype();

	virtual ActionValue construct(ActionContext* context, const ActionValueArray& args);

	void Elastic_easeIn(CallArgs& ca);

	void Elastic_easeInOut(CallArgs& ca);

	void Elastic_easeOut(CallArgs& ca);
};

	}
}

#endif	// traktor_flash_As_mx_transitions_easing_Elastic_H
