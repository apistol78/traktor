#ifndef traktor_script_IScriptDelegate_H
#define traktor_script_IScriptDelegate_H

#include "Core/Object.h"
#include "Script/Any.h"

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

class T_DLLCLASS IScriptDelegate : public Object
{
	T_RTTI_CLASS;

public:
	virtual Any call(int32_t argc, const Any* argv) = 0;
};

	}
}

#endif	// traktor_script_IScriptDelegate_H
