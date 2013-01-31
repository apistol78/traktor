#ifndef traktor_script_Delegate_H
#define traktor_script_Delegate_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCRIPT_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace script
	{

class Any;
class IScriptContext;

/*! \brief Script delegate.
 * \ingroup Script
 */
class T_DLLCLASS Delegate : public Object
{
	T_RTTI_CLASS;

public:
	Delegate(IScriptContext* context, const std::string& methodName);

	Delegate(IScriptContext* context, Object* object, const std::string& methodName);

	Any invoke(uint32_t argc, const Any* argv);

private:
	Ref< IScriptContext > m_context;
	Ref< Object > m_object;
	std::string m_methodName;
};

void T_DLLCLASS registerDelegateClasses(class IScriptManager* scriptManager);

	}
}

#endif	// traktor_script_Delegate_H
