#ifndef traktor_flash_AsSound_H
#define traktor_flash_AsSound_H

#include "Flash/Action/ActionClass.h"

namespace traktor
{
	namespace flash
	{

class ActionVM;
class ActionContext;
struct CallArgs;

/*! \brief Sound class.
 * \ingroup Flash
 */
class AsSound : public ActionClass
{
	T_RTTI_CLASS;

public:
	static Ref< AsSound > getInstance();

private:
	AsSound();

	void createPrototype();

	virtual ActionValue construct(ActionContext* context, const args_t& args);

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
