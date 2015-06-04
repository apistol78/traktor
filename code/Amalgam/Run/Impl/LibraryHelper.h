#ifndef traktor_amalgam_LibraryHelper_H
#define traktor_amalgam_LibraryHelper_H

#include "Core/Object.h"

namespace traktor
{
	namespace amalgam
	{

Ref< Object > loadAndInstantiate(const std::wstring& typeName);

template < typename Type >
Ref< Type > loadAndInstantiate(const std::wstring& typeName)
{
	Ref< Object > object = loadAndInstantiate(typeName);
	return dynamic_type_cast< Type* >(object);
}

	}
}

#endif	// traktor_amalgam_LibraryHelper_H
