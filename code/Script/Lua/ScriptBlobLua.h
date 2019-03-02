#pragma once

#include "Core/Misc/AutoPtr.h"
#include "Script/IScriptBlob.h"

namespace traktor
{
	namespace script
	{

class ScriptBlobLua : public IScriptBlob
{
	T_RTTI_CLASS;

public:
	virtual void serialize(ISerializer& s) override final;

private:
	friend class ScriptContextLua;
	friend class ScriptManagerLua;

	std::string m_fileName;
	std::string m_script;
};

	}
}

