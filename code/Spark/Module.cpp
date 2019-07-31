#include "Core/Rtti/TypeInfo.h"

#if defined(T_STATIC)
#	include "Spark/ClassFactory.h"
#	include "Spark/Action/Avm1/ActionVMImage1.h"
#	include "Spark/Action/Avm2/ActionVMImage2.h"

namespace traktor
{
	namespace spark
	{

extern "C" void __module__Traktor_Flash()
{
	T_FORCE_LINK_REF(ClassFactory);
	T_FORCE_LINK_REF(ActionVMImage1);
	T_FORCE_LINK_REF(ActionVMImage2);
}

	}
}

#endif
