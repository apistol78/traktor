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

	virtual void init(ActionObject* self, const ActionValueArray& args);

	virtual void coerce(ActionObject* self) const;

	void initialize(ActionObject* subject);

private:
	void AsBroadcaster_initialize(CallArgs& ca);

	void AsBroadcaster_addListener(CallArgs& ca);

	void AsBroadcaster_broadcastMessage(CallArgs& ca);

	void AsBroadcaster_removeListener(CallArgs& ca);
};

	}
}

#endif	// traktor_flash_AsAsBroadcaster_H
