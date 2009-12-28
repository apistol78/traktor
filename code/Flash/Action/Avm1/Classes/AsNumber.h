#ifndef traktor_flash_AsNumber_H
#define traktor_flash_AsNumber_H

#include "Flash/Action/Avm1/ActionClass.h"

namespace traktor
{
	namespace flash
	{

struct CallArgs;

/*! \brief Number class.
 * \ingroup Flash
 */
class AsNumber : public ActionClass
{
	T_RTTI_CLASS;

public:
	static Ref< AsNumber > getInstance();

private:
	AsNumber();

	void createPrototype();

	virtual ActionValue construct(ActionContext* context, const args_t& args);

	void Number_toString(CallArgs& ca);

	void Number_valueOf(CallArgs& ca);
};

	}
}

#endif	// traktor_flash_AsNumber_H
