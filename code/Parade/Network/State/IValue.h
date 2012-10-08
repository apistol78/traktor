#ifndef traktor_parade_IValue_H
#define traktor_parade_IValue_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PARADE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace parade
	{

class T_DLLCLASS IValue : public Object
{
	T_RTTI_CLASS;
};

	}
}

#endif	// traktor_parade_IValue_H
