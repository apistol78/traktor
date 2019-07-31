#pragma once

#include "Spark/Action/ActionClass.h"

namespace traktor
{
	namespace spark
	{

struct CallArgs;

/*! \brief AsBroadcaster class.
 * \ingroup Spark
 */
class AsAsBroadcaster : public ActionClass
{
	T_RTTI_CLASS;

public:
	AsAsBroadcaster(ActionContext* context);

	virtual void initialize(ActionObject* self) override final;

	virtual void construct(ActionObject* self, const ActionValueArray& args) override final;

	virtual ActionValue xplicit(const ActionValueArray& args) override final;

	void initializeSubject(ActionObject* subject, bool useWeakReferences);

private:
	void AsBroadcaster_initialize(CallArgs& ca);

	void AsBroadcaster_addListener(CallArgs& ca);

	void AsBroadcaster_addListenerWeak(CallArgs& ca);

	void AsBroadcaster_broadcastMessage(CallArgs& ca);

	void AsBroadcaster_removeListener(CallArgs& ca);
};

	}
}

