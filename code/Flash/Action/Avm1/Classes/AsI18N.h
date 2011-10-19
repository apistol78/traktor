#ifndef traktor_flash_AsI18N_H
#define traktor_flash_AsI18N_H

#include "Core/RefArray.h"
#include "Flash/Action/Avm1/ActionClass.h"

namespace traktor
{
	namespace flash
	{

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
	AsI18N();

	virtual Ref< ActionObject > alloc(ActionContext* context);

	virtual void init(ActionContext* context, ActionObject* self, const ActionValueArray& args);

private:
	void I18N_format(CallArgs& ca);
};

	}
}

#endif	// traktor_flash_AsI18N_H
