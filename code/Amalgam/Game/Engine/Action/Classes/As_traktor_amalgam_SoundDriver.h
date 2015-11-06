#ifndef traktor_amalgam_As_traktor_amalgam_SoundDriver_h
#define traktor_amalgam_As_traktor_amalgam_SoundDriver_h

#include "Flash/Action/ActionClass.h"
#include "Flash/Action/ActionFunctionNative.h"

namespace traktor
{
	namespace amalgam
	{

class AsSoundDriver;

/*! \brief Traktor sound driver descriptor ActionScript wrapper.
 * \ingroup Amalgam
 */
class As_traktor_amalgam_SoundDriver : public flash::ActionClass
{
	T_RTTI_CLASS;

public:
	As_traktor_amalgam_SoundDriver(flash::ActionContext* context);

	virtual void initialize(flash::ActionObject* self) T_OVERRIDE T_FINAL;

	virtual void construct(flash::ActionObject* self, const flash::ActionValueArray& args) T_OVERRIDE T_FINAL;

	virtual flash::ActionValue xplicit(const flash::ActionValueArray& args) T_OVERRIDE T_FINAL;

private:
	void SoundDriver_get_available(flash::CallArgs& ca);

	std::wstring SoundDriver_get_name(const AsSoundDriver* self) const;
};

	}
}

#endif	// traktor_amalgam_As_traktor_amalgam_SoundDriver_h
