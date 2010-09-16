#ifndef traktor_flash_AsFunction_H
#define traktor_flash_AsFunction_H

#include "Flash/Action/Avm1/ActionClass.h"

namespace traktor
{
	namespace flash
	{

struct CallArgs;

/*! \brief Function class.
 * \ingroup Flash
 */
class AsFunction : public ActionClass
{
	T_RTTI_CLASS;

public:
	AsFunction();

private:
	virtual ActionValue construct(ActionContext* context, const ActionValueArray& args);

	void Function_apply(CallArgs& ca);

	void Function_call(CallArgs& ca);
};

	}
}

#endif	// traktor_flash_AsFunction_H
