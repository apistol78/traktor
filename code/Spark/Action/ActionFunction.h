#pragma once

#include "Spark/Action/ActionObject.h"
#include "Spark/Action/ActionValueArray.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace spark
	{

class ActionContext;
class ActionFrame;
class ActionValue;

/*! \brief ActionScript callable function base.
 * \ingroup Spark
 */
class T_DLLCLASS ActionFunction : public ActionObject
{
	T_RTTI_CLASS;

public:
	ActionFunction(ActionContext* context, const char* name);

	virtual ActionValue call(ActionObject* self, ActionObject* super, const ActionValueArray& args) = 0;

	ActionValue call(ActionObject* self) { return call(self, 0, ActionValueArray()); }

	ActionValue call(ActionObject* self, const ActionValueArray& args) { return call(self, 0, args); }

	ActionValue call(ActionObject* self, ActionObject* super) { return call(self, super, ActionValueArray()); }

	ActionValue call(const ActionValueArray& args) { return call(0, 0, args); }

	ActionValue call() { return call(0, 0, ActionValueArray()); }

	ActionValue call(ActionFrame* callerFrame, ActionObject* self, ActionObject* super);

	ActionValue call(ActionFrame* callerFrame, ActionObject* self);

	ActionValue call(ActionFrame* callerFrame);

#if defined(_DEBUG)
	void setName(const char* name) { m_name = name; }

	const std::string& getName() const { return m_name; }
#endif

private:
#if defined(_DEBUG)
	std::string m_name;
#endif
};

	}
}

