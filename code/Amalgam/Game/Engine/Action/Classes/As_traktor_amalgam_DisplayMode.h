#ifndef traktor_amalgam_As_traktor_amalgam_DisplayMode_h
#define traktor_amalgam_As_traktor_amalgam_DisplayMode_h

#include "Flash/Action/ActionClass.h"
#include "Flash/Action/ActionFunctionNative.h"

namespace traktor
{
	namespace amalgam
	{

class IEnvironment;

	}

	namespace amalgam
	{

class AsDisplayMode;

/*! \brief Traktor display mode class ActionScript wrapper.
 * \ingroup Amalgam
 */
class As_traktor_amalgam_DisplayMode : public flash::ActionClass
{
	T_RTTI_CLASS;

public:
	As_traktor_amalgam_DisplayMode(flash::ActionContext* context, IEnvironment* environment);

	virtual void initialize(flash::ActionObject* self) T_OVERRIDE T_FINAL;

	virtual void construct(flash::ActionObject* self, const flash::ActionValueArray& args) T_OVERRIDE T_FINAL;

	virtual flash::ActionValue xplicit(const flash::ActionValueArray& args) T_OVERRIDE T_FINAL;

private:
	IEnvironment* m_environment;

	void DisplayMode_get_availableModes(flash::CallArgs& ca);

	void DisplayMode_get_currentMode(flash::CallArgs& ca);

	uint32_t DisplayMode_get_width(const AsDisplayMode* self) const;

	uint32_t DisplayMode_get_height(const AsDisplayMode* self) const;

	uint32_t DisplayMode_get_refreshRate(const AsDisplayMode* self) const;

	uint32_t DisplayMode_get_colorBits(const AsDisplayMode* self) const;

	bool DisplayMode_get_stereoscopic(const AsDisplayMode* self) const;
};

	}
}

#endif	// traktor_amalgam_As_traktor_amalgam_DisplayMode_h
