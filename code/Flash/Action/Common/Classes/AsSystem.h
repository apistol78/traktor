#ifndef traktor_flash_AsSystem_H
#define traktor_flash_AsSystem_H

#include "Flash/Action/ActionObject.h"

namespace traktor
{
	namespace flash
	{

/*! \brief System class.
 * \ingroup Flash
 */
class AsSystem : public ActionObject
{
	T_RTTI_CLASS;

public:
	AsSystem(ActionContext* context);
};

	}
}

#endif	// traktor_flash_AsSystem_H
