#ifndef traktor_flash_AsBoolean_H
#define traktor_flash_AsBoolean_H

#include "Flash/Action/Avm1/ActionClass.h"

namespace traktor
{
	namespace flash
	{

struct CallArgs;

/*! \brief Boolean class.
 * \ingroup Flash
 */
class AsBoolean : public ActionClass
{
	T_RTTI_CLASS;

public:
	static Ref< AsBoolean > getInstance();

private:
	AsBoolean();

	void createPrototype();

	virtual ActionValue construct(ActionContext* context, const args_t& args);

	void Boolean_toString(CallArgs& ca);

	void Boolean_valueOf(CallArgs& ca);
};

	}
}

#endif	// traktor_flash_AsBoolean_H
