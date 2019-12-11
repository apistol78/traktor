#pragma once

#include "Spark/Action/ActionClass.h"

namespace traktor
{
	namespace spark
	{

struct CallArgs;

/*! Security class.
 * \ingroup Spark
 */
class AsSecurity : public ActionClass
{
	T_RTTI_CLASS;

public:
	AsSecurity(ActionContext* context);

	virtual void initialize(ActionObject* self) override final;

	virtual void construct(ActionObject* self, const ActionValueArray& args) override final;

	virtual ActionValue xplicit(const ActionValueArray& args) override final;

private:
	void Security_allowDomain(CallArgs& ca);

	void Security_allowInsecureDomain(CallArgs& ca);

	void Security_loadPolicyFile(CallArgs& ca);
};

	}
}

