#ifndef traktor_flash_AsTextField_H
#define traktor_flash_AsTextField_H

#include "Flash/Action/Avm1/ActionClass.h"

namespace traktor
{
	namespace flash
	{

struct CallArgs;

/*! \brief TextField class.
 * \ingroup Flash
 */
class AsTextField : public ActionClass
{
	T_RTTI_CLASS;

public:
	static Ref< AsTextField > getInstance();

private:
	AsTextField();

	void createPrototype();

	virtual ActionValue construct(ActionContext* context, const ActionValueArray& args);

	void TextField_get_text(CallArgs& ca);

	void TextField_set_text(CallArgs& ca);
};

	}
}

#endif	// traktor_flash_AsTextField_H
