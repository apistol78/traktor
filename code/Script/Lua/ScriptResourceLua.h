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
	ScriptResourceLua();

	virtual void serialize(ISerializer& s);

private:
	friend class ScriptManagerLua;

	std::string m_fileName;
	source_map_t m_map;
	bool m_precompiled;
	uint32_t m_bufferSize;
	AutoArrayPtr< uint8_t > m_buffer;
};

	}
}

#endif	// traktor_script_ScriptResourceLua_H
