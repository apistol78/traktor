#ifndef traktor_flash_ActionSuper_H
#define traktor_flash_ActionSuper_H

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

/*! \brief Class wrapper to access parent class members.
 * \ingroup Flash
 *
 * \note Super is accessed through
 * "(object).__proto__.__constructor__" and
 * "(object).__proto__.__proto__"
 * which are relationships that AopExtends prepares.
 */
class T_DLLCLASS ActionSuper : public ActionFunction
{
	T_RTTI_CLASS;

public:
	ActionSuper(ActionObject* object);

	virtual ActionValue call(ActionContext* context, ActionObject* self, const ActionValueArray& args);

	virtual ActionValue call(ActionFrame* callerFrame, ActionObject* self);

private:
	Ref< ActionObject > m_superClass;
	Ref< ActionObject > m_superPrototype;
	Ref< ActionObject > m_object;
};

	}
}

#endif	// traktor_flash_ActionSuper_H
