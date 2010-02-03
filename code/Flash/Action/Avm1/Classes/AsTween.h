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
};

	}
}

#endif	// traktor_flash_AsTween_H
