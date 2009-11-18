#ifndef traktor_flash_ActionClass_H
#define traktor_flash_ActionClass_H

#include "Flash/Action/ActionFunction.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
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
	typedef std::vector< ActionValue > args_t;

	ActionClass(const std::wstring& name);

	virtual ActionValue call(ActionVM* vm, ActionFrame* callerFrame, ActionObject* self);

protected:
	virtual ActionValue construct(ActionContext* context, const args_t& args) = 0;
};

	}
}

#endif	// traktor_flash_ActionClass_H
