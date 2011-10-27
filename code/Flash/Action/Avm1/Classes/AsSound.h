#ifndef traktor_flash_AsSound_H
#define traktor_flash_AsSound_H

#include "Flash/Action/Avm1/ActionClass.h"

namespace traktor
{
	namespace flash
	{

struct CallArgs;

/*! \brief Sound class.
 * \ingroup Flash
 */
class AsSound : public ActionClass
{
	T_RTTI_CLASS;

public:
	AsSound(ActionContext* context);

	virtual void initialize(ActionObject* self);

	virtual void construct(ActionObject* self, const ActionValueArray& args);

	virtual ActionValue xplicit(const ActionValueArray& args);

private:
	void Sound_attachSound(CallArgs& ca);

	void Sound_getBytesLoaded(CallArgs& ca);

	void Sound_getBytesTotal(CallArgs& ca);

	void Sound_getPan(CallArgs& ca);

	void Sound_getTransform(CallArgs& ca);

	void Sound_getVolume(CallArgs& ca);

	void Sound_loadSound(CallArgs& ca);

	void Sound_setPan(CallArgs& ca);

	void Sound_setTransform(CallArgs& ca);

	void Sound_setVolume(CallArgs& ca);

	void Sound_start(CallArgs& ca);

	void Sound_stop(CallArgs& ca);

	void Sound_get_checkPolicyFile(CallArgs& ca);

	void Sound_set_checkPolicyFile(CallArgs& ca);

	void Sound_get_duration(CallArgs& ca);

	void Sound_get_id3(CallArgs& ca);

	void Sound_get_position(CallArgs& ca);
};

	}
}

#endif	// traktor_flash_AsSound_H
