#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Json/JsonDocument.h"

namespace traktor
{
	namespace json
	{

extern "C" void __module__Traktor_Json()
{
	T_FORCE_LINK_REF(JsonDocument);
}

	}
}

#endif
