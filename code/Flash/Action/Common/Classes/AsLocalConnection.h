#pragma once

#include "Flash/Action/ActionClass.h"

namespace traktor
{
	namespace flash
	{

struct CallArgs;

/*! \brief LocalConnection class.
 * \ingroup Flash
 */
class AsLocalConnection : public ActionClass
{
	T_RTTI_CLASS;

public:
	AsLocalConnection(ActionContext* context);

	virtual void initialize(ActionObject* self) override final;

	virtual void construct(ActionObject* self, const ActionValueArray& args) override final;

	virtual ActionValue xplicit(const ActionValueArray& args) override final;

private:
	void LocalConnection_close(CallArgs& ca);

	void LocalConnection_connect(CallArgs& ca);

	void LocalConnection_domain(CallArgs& ca);

	void LocalConnection_send(CallArgs& ca);
};

	}
}

