#ifndef traktor_script_ScriptResourceLua_H
#define traktor_script_ScriptResourceLua_H

#include "Core/Containers/AlignedVector.h"
#include "Script/IScriptResource.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCRIPT_LUA_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace script
	{

class T_DLLCLASS ScriptResourceLua : public IScriptResource
{
	T_RTTI_CLASS;

public:
	ScriptResourceLua();

	ScriptResourceLua(const std::string& script);

	const std::string& getScript() const;

	virtual bool serialize(ISerializer& s);

private:
	std::string m_script;
};

	}
}

#endif	// traktor_script_ScriptResourceLua_H
