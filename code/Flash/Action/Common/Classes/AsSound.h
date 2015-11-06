#ifndef traktor_flash_AsSound_H
#define traktor_flash_AsSound_H

#include "Flash/Action/ActionClass.h"

namespace traktor
{
	namespace flash
	{

struct CallArgs;
class FlashSoundPlayer;

/*! \brief Sound class.
 * \ingroup Flash
 */
class AsSound : public ActionClass
{
	T_RTTI_CLASS;

public:
	AsSound(ActionContext* context, FlashSoundPlayer* soundPlayer);

	virtual void initialize(ActionObject* self) T_OVERRIDE T_FINAL;

	virtual void construct(ActionObject* self, const ActionValueArray& args) T_OVERRIDE T_FINAL;

	virtual ActionValue xplicit(const ActionValueArray& args) T_OVERRIDE T_FINAL;

private:
	Ref< FlashSoundPlayer > m_soundPlayer;

	void Sound_attachSound(ActionObject* self, const std::string& exportName) const;

	void Sound_getBytesLoaded(CallArgs& ca);

	void Sound_getBytesTotal(CallArgs& ca);

	void Sound_getPan(CallArgs& ca);

	void Sound_getTransform(CallArgs& ca);

	void Sound_getVolume(CallArgs& ca);

	void Sound_loadSound(CallArgs& ca);

	void Sound_setPan(CallArgs& ca);

	void Sound_setTransform(CallArgs& ca);

	void Sound_setVolume(CallArgs& ca);

	void Sound_start(ActionObject* self) const;

	void Sound_stop(ActionObject* self) const;

	void Sound_get_checkPolicyFile(CallArgs& ca);

	void Sound_set_checkPolicyFile(CallArgs& ca);

	void Sound_get_duration(CallArgs& ca);

	void Sound_get_id3(CallArgs& ca);

	void Sound_get_position(CallArgs& ca);
};

	}
}

#endif	// traktor_flash_AsSound_H
