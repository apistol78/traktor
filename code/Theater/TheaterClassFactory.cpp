#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Theater/TheaterClassFactory.h"
#include "Theater/TheaterController.h"

namespace traktor
{
	namespace theater
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.scene.TheaterClassFactory", 0, TheaterClassFactory, IRuntimeClassFactory)

void TheaterClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	auto classTheaterController = new AutoRuntimeClass< TheaterController >();
	classTheaterController->addMethod("play", &TheaterController::play);
	registrar->registerClass(classTheaterController);
}

	}
}
