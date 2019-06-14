#include "Core/Rtti/ITypedObject.h"

namespace traktor
{

const TypeInfo ITypedObject::ms_typeInfo(
	L"traktor.ITypedObject",
	sizeof(ITypedObject),
	0,
	false,
	0,
	0
);

const TypeInfo& ITypedObject::getClassTypeInfo()
{
	return ms_typeInfo;
}

}
