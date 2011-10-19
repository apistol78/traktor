#ifndef traktor_flash_As_mx_transitions_Tween_H
#define traktor_flash_As_mx_transitions_Tween_H

#include "Flash/Action/Avm1/ActionClass.h"

namespace traktor
{
	namespace flash
	{

/*! \brief mx.transitions.Tween class.
 * \ingroup Flash
 */
class As_mx_transitions_Tween : public ActionClass
{
	T_RTTI_CLASS;

public:
	As_mx_transitions_Tween();

	virtual Ref< ActionObject > alloc(ActionContext* context);

	virtual void init(ActionContext* context, ActionObject* self, const ActionValueArray& args);
};

	}
}

#endif	// traktor_flash_As_mx_transitions_Tween_H
