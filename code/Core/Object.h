#ifndef traktor_Object_H
#define traktor_Object_H

#include "Core/Rtti/ITypedObject.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*
*/
class T_DLLCLASS Object : public RefCountImpl< ITypedObject >
{
	T_RTTI_CLASS;
};	

}

#endif	// traktor_Object_H
