#pragma once

#include "Flash/Action/ActionClass.h"

namespace traktor
{
	namespace flash
	{

struct CallArgs;

/*! \brief Error class.
 * \ingroup Flash
 */
class AsError : public ActionClass
{
	T_RTTI_CLASS;

public:
	AsError(ActionContext* context);

	virtual void initialize(ActionObject* self) override final;

	virtual void construct(ActionObject* self, const ActionValueArray& args) override final;

	virtual ActionValue xplicit(const ActionValueArray& args) override final;

private:
	void Error_toString(CallArgs& ca);
};

	}
}

