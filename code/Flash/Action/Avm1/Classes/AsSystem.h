#ifndef traktor_flash_AsSystem_H
#define traktor_flash_AsSystem_H

#include "Flash/Action/Avm1/ActionClass.h"

namespace traktor
{
	namespace flash
	{

struct CallArgs;

/*! \brief System class.
 * \ingroup Flash
 */
class AsSystem : public ActionClass
{
	T_RTTI_CLASS;

public:
	AsSystem(ActionContext* context);

	virtual void init(ActionObject* self, const ActionValueArray& args);

	virtual void coerce(ActionObject* self) const;
};

	}
}

#endif	// traktor_flash_AsSystem_H
