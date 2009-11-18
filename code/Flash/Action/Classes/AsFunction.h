#ifndef traktor_flash_AsFunction_H
#define traktor_flash_AsFunction_H

#include "Flash/Action/ActionClass.h"

namespace traktor
{
	namespace flash
	{

/*! \brief Function class.
 * \ingroup Flash
 */
class AsFunction : public ActionClass
{
	T_RTTI_CLASS;

public:
	static Ref< AsFunction > getInstance();

private:
	AsFunction();

	void createPrototype();

	virtual ActionValue construct(ActionContext* context, const args_t& args);
};

	}
}

#endif	// traktor_flash_AsFunction_H
