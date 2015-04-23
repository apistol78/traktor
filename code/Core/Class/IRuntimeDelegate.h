#ifndef traktor_IScriptDelegate_H
#define traktor_IScriptDelegate_H

#include "Core/Object.h"
#include "Core/Class/Any.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief
 * \ingroup Core
 */
class T_DLLCLASS IRuntimeDelegate : public Object
{
	T_RTTI_CLASS;

public:
	virtual Any call(int32_t argc, const Any* argv) = 0;
};

}

#endif	// traktor_IScriptDelegate_H
