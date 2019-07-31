#pragma once

#include "Spark/Action/ActionClass.h"

namespace traktor
{
	namespace spark
	{

struct CallArgs;

/*! \brief Function class.
 * \ingroup Spark
 */
class AsFunction : public ActionClass
{
	T_RTTI_CLASS;

public:
	AsFunction(ActionContext* context);

	virtual void initialize(ActionObject* self) override final;

	virtual void construct(ActionObject* self, const ActionValueArray& args) override final;

	virtual ActionValue xplicit(const ActionValueArray& args) override final;

private:
	void Function_apply(CallArgs& ca);

	void Function_call(CallArgs& ca);

	void Function_toString(CallArgs& ca);
};

	}
}

