#ifndef traktor_amalgam_As_traktor_amalgam_I18N_H
#define traktor_amalgam_As_traktor_amalgam_I18N_H

#include "Core/RefArray.h"
#include "Flash/Action/ActionObject.h"

namespace traktor
{
	namespace flash
	{

struct CallArgs;

	}

	namespace amalgam
	{

/*! \brief I18N class.
 * \ingroup Amalgam
 */
class As_traktor_amalgam_I18N : public flash::ActionObject
{
	T_RTTI_CLASS;

public:
	As_traktor_amalgam_I18N(flash::ActionContext* context);

private:
	void I18N_format(flash::CallArgs& ca);
};

	}
}

#endif	// traktor_amalgam_As_traktor_amalgam_I18N_H
