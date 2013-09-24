#ifndef traktor_editor_ScriptProcessor_H
#define traktor_editor_ScriptProcessor_H

#include "Core/Object.h"

namespace traktor
{
	namespace script
	{

class IScriptManager;

	}

	namespace editor
	{

class ScriptProcessor : public Object
{
	T_RTTI_CLASS;

public:
	bool create();

	void destroy();

	bool generateFromFile(const std::wstring& fileName, std::wstring& output) const;

	bool generateFromSource(const std::wstring& source, std::wstring& output) const;

private:
	Ref< script::IScriptManager > m_scriptManager;
};

	}
}

#endif	// traktor_editor_ScriptProcessor_H
