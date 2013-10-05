#ifndef traktor_amalgam_As_traktor_amalgam_InputFabricator_H
#define traktor_amalgam_As_traktor_amalgam_InputFabricator_H

#include "Amalgam/IEnvironment.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Avm1/ActionClass.h"

namespace traktor
{
	namespace amalgam
	{

class As_traktor_amalgam_InputFabricator : public flash::ActionClass
{
	T_RTTI_CLASS;

public:
	As_traktor_amalgam_InputFabricator(flash::ActionContext* context, amalgam::IEnvironment* environment);

	virtual void initialize(flash::ActionObject* self);

	virtual void construct(flash::ActionObject* self, const flash::ActionValueArray& args);

	virtual flash::ActionValue xplicit(const flash::ActionValueArray& args);

private:
	amalgam::IEnvironment* m_environment;

	void InputFabricator_fabricateSource(flash::CallArgs& ca);

	void InputFabricator_apply(flash::CallArgs& ca);

	void InputFabricator_revert(flash::CallArgs& ca);
};

	}
}

#endif	// traktor_amalgam_As_traktor_amalgam_InputFabricator_H
