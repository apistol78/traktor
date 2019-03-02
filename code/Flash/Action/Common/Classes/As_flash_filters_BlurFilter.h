#pragma once

#include "Flash/Action/ActionClass.h"

namespace traktor
{
	namespace flash
	{

/*! \brief
 * \ingroup Flash
 */
class As_flash_filters_BlurFilter : public ActionClass
{
	T_RTTI_CLASS;

public:
	As_flash_filters_BlurFilter(ActionContext* context);

	virtual void initialize(ActionObject* self) override final;

	virtual void construct(ActionObject* self, const ActionValueArray& args) override final;

	virtual ActionValue xplicit(const ActionValueArray& args) override final;
};

	}
}

