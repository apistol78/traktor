#pragma once

#include "Spark/Action/ActionClass.h"

namespace traktor
{
	namespace spark
	{

/*! \brief Rectangle class.
 * \ingroup Spark
 */
class As_flash_geom_Rectangle : public ActionClass
{
	T_RTTI_CLASS;

public:
	As_flash_geom_Rectangle(ActionContext* context);

	virtual void initialize(ActionObject* self) override final;

	virtual void construct(ActionObject* self, const ActionValueArray& args) override final;

	virtual ActionValue xplicit(const ActionValueArray& args) override final;
};

	}
}

