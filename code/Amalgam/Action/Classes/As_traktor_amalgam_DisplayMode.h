#ifndef traktor_amalgam_As_traktor_amalgam_DisplayMode_h
#define traktor_amalgam_As_traktor_amalgam_DisplayMode_h

#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Avm1/ActionClass.h"

namespace traktor
{
	namespace amalgam
	{

class IEnvironment;

	}

	namespace amalgam
	{

class AsDisplayMode;

class As_traktor_amalgam_DisplayMode : public flash::ActionClass
{
	T_RTTI_CLASS;

public:
	As_traktor_amalgam_DisplayMode(flash::ActionContext* context, amalgam::IEnvironment* environment);

	virtual void initialize(flash::ActionObject* self);

	virtual void construct(flash::ActionObject* self, const flash::ActionValueArray& args);

	virtual flash::ActionValue xplicit(const flash::ActionValueArray& args);

private:
	amalgam::IEnvironment* m_environment;

	void DisplayMode_get_availableModes(flash::CallArgs& ca);

	uint32_t DisplayMode_get_width(const AsDisplayMode* self) const;

	uint32_t DisplayMode_get_height(const AsDisplayMode* self) const;

	uint32_t DisplayMode_get_refreshRate(const AsDisplayMode* self) const;

	uint32_t DisplayMode_get_colorBits(const AsDisplayMode* self) const;

	bool DisplayMode_get_stereoscopic(const AsDisplayMode* self) const;
};

	}
}

#endif	// traktor_amalgam_As_traktor_amalgam_DisplayMode_h
