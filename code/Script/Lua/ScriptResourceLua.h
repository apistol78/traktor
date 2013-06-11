#ifndef traktor_script_ScriptResourceLua_H
#define traktor_script_ScriptResourceLua_H

#include "Core/Misc/AutoPtr.h"
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

	virtual void serialize(ISerializer& s);

private:
	friend class ScriptManagerLua;

	std::string m_fileName;
	source_map_t m_map;
	uint32_t m_bufferSize;
	AutoArrayPtr< uint8_t > m_buffer;
};

	}
}

#endif	// traktor_script_ScriptResourceLua_H
