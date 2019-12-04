#include "Compress/ClassFactory.h"
#include "Compress/Zip/ZipVolume.h"
#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Core/Class/IRuntimeDelegate.h"
#include "Core/Io/IStream.h"

namespace traktor
{
	namespace compress
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.compress.ClassFactory", 0, ClassFactory, IRuntimeClassFactory)

void ClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	auto classZipVolume = new AutoRuntimeClass< ZipVolume >();
	classZipVolume->addConstructor< IStream* >();
	registrar->registerClass(classZipVolume);
}

	}
}
