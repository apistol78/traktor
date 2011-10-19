#ifndef traktor_flash_ActionClass_H
#define traktor_flash_ActionClass_H

#include "Flash/Action/ActionFunction.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

class ActionContext;

/*! \brief Base class for all native ActionScript classes.
 * \ingroup Flash
 */
class T_DLLCLASS ActionClass : public ActionFunction
{
	T_RTTI_CLASS;

public:
	ActionClass(ActionContext* context, const std::string& className);

	virtual ActionValue call(ActionObject* self, const ActionValueArray& args);

	virtual ActionValue call(ActionFrame* callerFrame, ActionObject* self);

	virtual void init(ActionObject* self, const ActionValueArray& args) const = 0;

	virtual void coerce(ActionObject* self) const = 0;
};

	}
}

#endif	// traktor_flash_ActionClass_H
