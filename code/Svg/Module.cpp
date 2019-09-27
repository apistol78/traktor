#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Svg/ClassFactory.h"
#	include "Svg/Document.h"
#	include "Svg/Parser.h"

namespace traktor
{
	namespace svg
	{

extern "C" void __module__Traktor_Svg()
{
	T_FORCE_LINK_REF(ClassFactory);
	T_FORCE_LINK_REF(Document);
	T_FORCE_LINK_REF(Parser);
}

	}
}

#endif
