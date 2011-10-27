#ifndef traktor_flash_AsI18N_H
#define traktor_flash_AsI18N_H

#include "Core/RefArray.h"
#include "Flash/Action/ActionObject.h"

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
class AsI18N : public ActionObject
{
	T_RTTI_CLASS;

public:
	AsI18N(ActionContext* context);

private:
	void I18N_format(CallArgs& ca);
};

	}
}

#endif	// traktor_flash_AsI18N_H
