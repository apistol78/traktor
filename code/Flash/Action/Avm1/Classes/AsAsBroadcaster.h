#ifndef traktor_flash_AsAsBroadcaster_H
#define traktor_flash_AsAsBroadcaster_H

#include "Flash/Action/Avm1/ActionClass.h"

namespace traktor
{
	namespace flash
	{

struct CallArgs;

/*! \brief AsBroadcaster class.
 * \ingroup Flash
 */
class AsAsBroadcaster : public ActionClass
{
	T_RTTI_CLASS;

public:
	AsAsBroadcaster(ActionContext* context);

	virtual void initialize(ActionObject* self);

	virtual void construct(ActionObject* self, const ActionValueArray& args);

	virtual ActionValue xplicit(const ActionValueArray& args);

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

#endif	// traktor_flash_AsAsBroadcaster_H
