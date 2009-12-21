#ifndef traktor_flash_AsI18N_H
#define traktor_flash_AsI18N_H

#include "Core/RefArray.h"
#include "Flash/Action/ActionClass.h"

namespace traktor
{
	namespace flash
	{

class ActionVM;
class ActionContext;
struct CallArgs;

/*! \brief I18N class.
 * \ingroup Flash
 *
 * This class doesn't exist in ActionScript specification
 * but are an extension provided by Traktor in order to
 * support heterogeneous localization.
 */
class AsI18N : public ActionClass
{
	T_RTTI_CLASS;

public:
	static Ref< AsI18N > getInstance();

	AsI18N();

private:
	void createPrototype();

	virtual ActionValue construct(ActionContext* context, const args_t& args);

	void I18N_format(CallArgs& ca);
};

	}
}

#endif	// traktor_flash_AsI18N_H
