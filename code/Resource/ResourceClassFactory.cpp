#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Resource/IResourceFactory.h"
#include "Resource/IResourceManager.h"
#include "Resource/ResourceClassFactory.h"

namespace traktor
{
	namespace resource
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.resource.ResourceClassFactory", 0, ResourceClassFactory, IRuntimeClassFactory)

void ResourceClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	Ref< AutoRuntimeClass< IResourceFactory > > classIResourceFactory = new AutoRuntimeClass< IResourceFactory >();
	classIResourceFactory->addMethod("isCacheable", &IResourceFactory::isCacheable);
	registrar->registerClass(classIResourceFactory);

	Ref< AutoRuntimeClass< ResourceHandle > > classIResourceHandle = new AutoRuntimeClass< ResourceHandle >();
	classIResourceHandle->addMethod("replace", &ResourceHandle::replace);
	classIResourceHandle->addMethod("get", &ResourceHandle::get);
	classIResourceHandle->addMethod("flush", &ResourceHandle::flush);
	registrar->registerClass(classIResourceHandle);

	Ref< AutoRuntimeClass< IResourceManager > > classIResourceManager = new AutoRuntimeClass< IResourceManager >();
	classIResourceManager->addMethod("addFactory", &IResourceManager::addFactory);
	classIResourceManager->addMethod("removeFactory", &IResourceManager::removeFactory);
	classIResourceManager->addMethod("removeAllFactories", &IResourceManager::removeAllFactories);
	registrar->registerClass(classIResourceManager);
}

	}
}
