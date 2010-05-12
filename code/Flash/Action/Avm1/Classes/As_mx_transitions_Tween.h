#ifndef traktor_flash_As_mx_transitions_Tween_H
#define traktor_flash_As_mx_transitions_Tween_H

#include "Flash/Action/Avm1/ActionClass.h"

namespace traktor
{
	namespace flash
	{

struct CallArgs;

/*! \brief mx.transitions.Tween class.
 * \ingroup Flash
 */
class As_mx_transitions_Tween : public ActionClass
{
	T_RTTI_CLASS;

public:
	static Ref< As_mx_transitions_Tween > getInstance();

private:
	As_mx_transitions_Tween();

	void createPrototype();

	virtual ActionValue construct(ActionContext* context, const ActionValueArray& args);

	void Tween_continueTo(CallArgs& ca);

	void Tween_fforward(CallArgs& ca);

	void Tween_nextFrame(CallArgs& ca);

	void Tween_prevFrame(CallArgs& ca);

	void Tween_resume(CallArgs& ca);

	void Tween_rewind(CallArgs& ca);

	void Tween_start(CallArgs& ca);

	void Tween_stop(CallArgs& ca);

	void Tween_yoyo(CallArgs& ca);
};

	}
}

#endif	// traktor_flash_As_mx_transitions_Tween_H
