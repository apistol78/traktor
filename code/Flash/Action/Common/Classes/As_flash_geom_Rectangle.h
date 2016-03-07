#ifndef traktor_flash_As_flash_geom_Rectangle_H
#define traktor_flash_As_flash_geom_Rectangle_H

#include "Flash/Action/ActionClass.h"

namespace traktor
{
	namespace flash
	{

/*! \brief Rectangle class.
 * \ingroup Flash
 */
class As_flash_geom_Rectangle : public ActionClass
{
	T_RTTI_CLASS;

public:
	As_flash_geom_Rectangle(ActionContext* context);

	virtual void initialize(ActionObject* self) T_OVERRIDE T_FINAL;

	virtual void construct(ActionObject* self, const ActionValueArray& args) T_OVERRIDE T_FINAL;

	virtual ActionValue xplicit(const ActionValueArray& args) T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_flash_As_flash_geom_Rectangle_H
