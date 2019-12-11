#pragma once

#include "Spark/Action/ActionClass.h"

namespace traktor
{
	namespace spark
	{

/*! mx.transitions.Tween class.
 * \ingroup Spark
 */
class As_mx_transitions_Tween : public ActionClass
{
	T_RTTI_CLASS;

public:
	As_mx_transitions_Tween(ActionContext* context);

	virtual void initialize(ActionObject* self) override final;

	virtual void construct(ActionObject* self, const ActionValueArray& args) override final;

	virtual ActionValue xplicit(const ActionValueArray& args) override final;
};

	}
}

