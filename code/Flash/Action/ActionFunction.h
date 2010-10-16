#ifndef traktor_flash_ActionFunction_H
#define traktor_flash_ActionFunction_H

#include "Flash/Action/ActionObject.h"

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
class ActionObject;
class ActionFrame;
class ActionValue;
class ActionValueArray;

/*! \brief ActionScript callable function base.
 * \ingroup Flash
 */
class T_DLLCLASS ActionFunction : public ActionObject
{
	T_RTTI_CLASS;

public:
	ActionFunction(const std::string& name);

	virtual ActionValue call(ActionContext* context, ActionObject* self, const ActionValueArray& args) = 0;

	virtual ActionValue call(ActionFrame* callerFrame, ActionObject* self) = 0;

	virtual ActionValue toString() const;

	void setName(const std::string& name) { m_name = name; }

	const std::string& getName() const { return m_name; }

private:
	std::string m_name;
};

	}
}

#endif	// traktor_flash_ActionFunction_H
