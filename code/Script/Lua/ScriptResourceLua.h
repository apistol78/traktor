#ifndef traktor_script_ScriptResourceLua_H
#define traktor_script_ScriptResourceLua_H

#include "Core/Containers/AlignedVector.h"
#include "Script/IScriptResource.h"
#include "Script/Types.h"

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

	ScriptResourceLua(const std::string& fileName, const std::string& script, const source_map_t& map);

	const std::string& getFileName() const;

	const std::string& getScript() const;

	const source_map_t& getMap() const;

	virtual void serialize(ISerializer& s);

private:
	std::string m_fileName;
	std::string m_script;
	source_map_t m_map;
};

	}
}

#endif	// traktor_script_ScriptResourceLua_H
