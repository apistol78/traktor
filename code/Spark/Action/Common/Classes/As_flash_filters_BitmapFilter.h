#pragma once

#include "Spark/Action/ActionClass.h"

namespace traktor
{
	namespace spark
	{

/*! BitmapFilter class.
 * \ingroup Spark
 */
class As_flash_filters_BitmapFilter : public ActionClass
{
	T_RTTI_CLASS;

public:
	As_flash_filters_BitmapFilter(ActionContext* context);

	virtual void initialize(ActionObject* self) override final;

	virtual void construct(ActionObject* self, const ActionValueArray& args) override final;

	virtual ActionValue xplicit(const ActionValueArray& args) override final;
};

	}
}

