#include <Core/Class/AutoRuntimeClass.h>
#include <Core/Class/Boxes.h>
#include <Core/Class/IRuntimeClassRegistrar.h>
#include "Shared/ClassFactory.h"
#include "Shared/QuadComponent.h"

using namespace traktor;

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"ClassFactory", 0, ClassFactory, IRuntimeClassFactory)

void ClassFactory::createClasses(traktor::IRuntimeClassRegistrar* registrar) const
{
	Ref< AutoRuntimeClass< QuadComponent > > classQuadComponent = new AutoRuntimeClass< QuadComponent >();
	classQuadComponent->addMethod("setColor", &QuadComponent::setColor);
	registrar->registerClass(classQuadComponent);
}
