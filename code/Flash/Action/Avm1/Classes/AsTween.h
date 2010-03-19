#ifndef traktor_flash_AsTween_H
#define traktor_flash_AsTween_H

#include "Flash/Action/Avm1/ActionClass.h"

namespace traktor
{
	namespace flash
	{

struct CallArgs;

/*! \brief mx.transitions.Tween class.
 * \ingroup Flash
 */
class AsTween : public ActionClass
{
	T_RTTI_CLASS;

public:
	static Ref< AsTween > getInstance();

private:
	AsTween();

	void createPrototype();

	virtual ActionValue construct(ActionContext* context, const args_t& args);

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

#endif	// traktor_flash_AsTween_H
