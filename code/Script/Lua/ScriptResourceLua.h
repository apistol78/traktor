#ifndef traktor_script_ScriptResourceLua_H
#define traktor_script_ScriptResourceLua_H

#include "Core/Misc/AutoPtr.h"
#include "Script/IScriptResource.h"
#include "Script/Types.h"

namespace traktor
{
	namespace script
	{

class ScriptResourceLua : public IScriptResource
{
	T_RTTI_CLASS;

public:
	virtual void serialize(ISerializer& s);

private:
	friend class ScriptContextLua;
	friend class ScriptManagerLua;

	std::string m_fileName;
	source_map_t m_map;
	std::string m_script;
};

	}
}

#endif	// traktor_script_ScriptResourceLua_H
