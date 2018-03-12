#include "Core/Rtti/ITypedObject.h"

namespace traktor
{
	namespace
	{

TypeInfo s_ITypedObject_typeInfo(
	L"traktor.ITypedObject",
	sizeof(ITypedObject),
	0,
	false,
	0,
	0
);

	}

const TypeInfo& ITypedObject::getClassTypeInfo()
{
	return s_ITypedObject_typeInfo;
}

}
