#include "Amalgam/Engine/Classes/ResourceClasses.h"
#include "Resource/IResourceFactory.h"
#include "Resource/IResourceManager.h"
#include "Script/AutoScriptClass.h"
#include "Script/IScriptManager.h"

namespace traktor
{
	namespace amalgam
	{

void registerResourceClasses(script::IScriptManager* scriptManager)
{
	Ref< script::AutoScriptClass< resource::IResourceFactory > > classIResourceFactory = new script::AutoScriptClass< resource::IResourceFactory >();
	classIResourceFactory->addMethod("isCacheable", &resource::IResourceFactory::isCacheable);
	scriptManager->registerClass(classIResourceFactory);

	Ref< script::AutoScriptClass< resource::IResourceHandle > > classIResourceHandle = new script::AutoScriptClass< resource::IResourceHandle >();
	classIResourceHandle->addMethod("replace", &resource::IResourceHandle::replace);
	classIResourceHandle->addMethod("get", &resource::IResourceHandle::get);
	classIResourceHandle->addMethod("flush", &resource::IResourceHandle::flush);
	scriptManager->registerClass(classIResourceHandle);

	Ref< script::AutoScriptClass< resource::IResourceManager > > classIResourceManager = new script::AutoScriptClass< resource::IResourceManager >();
	classIResourceManager->addMethod("addFactory", &resource::IResourceManager::addFactory);
	classIResourceManager->addMethod("removeFactory", &resource::IResourceManager::removeFactory);
	classIResourceManager->addMethod("removeAllFactories", &resource::IResourceManager::removeAllFactories);
	scriptManager->registerClass(classIResourceManager);
}

	}
}
