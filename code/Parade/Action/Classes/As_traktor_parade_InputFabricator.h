#ifndef traktor_parade_As_traktor_parade_InputFabricator_H
#define traktor_parade_As_traktor_parade_InputFabricator_H

#include "Amalgam/IEnvironment.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Avm1/ActionClass.h"

namespace traktor
{
	namespace parade
	{

class As_traktor_parade_InputFabricator : public flash::ActionClass
{
	T_RTTI_CLASS;

public:
	As_traktor_parade_InputFabricator(flash::ActionContext* context, amalgam::IEnvironment* environment);

	virtual void initialize(flash::ActionObject* self);

	virtual void construct(flash::ActionObject* self, const flash::ActionValueArray& args);

	virtual flash::ActionValue xplicit(const flash::ActionValueArray& args);

private:
	amalgam::IEnvironment* m_environment;

	void InputFabricator_fabricateSource(flash::CallArgs& ca);
};

	}
}

#endif	// traktor_parade_As_traktor_parade_InputFabricator_H
