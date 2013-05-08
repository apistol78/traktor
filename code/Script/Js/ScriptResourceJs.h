#ifndef traktor_script_ScriptResourceJs_H
#define traktor_script_ScriptResourceJs_H

#include "Core/Containers/AlignedVector.h"
#include "Script/IScriptResource.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCRIPT_JS_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace script
	{

class T_DLLCLASS ScriptResourceJs : public IScriptResource
{
	T_RTTI_CLASS;

public:
	ScriptResourceJs();

	ScriptResourceJs(const std::wstring& script);

	const std::wstring& getScript() const;

	virtual void serialize(ISerializer& s);

private:
	std::wstring m_script;
};

	}
}

#endif	// traktor_script_ScriptResourceJs_H
