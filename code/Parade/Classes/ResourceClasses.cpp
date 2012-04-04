#include "Parade/Classes/ResourceClasses.h"
#include "Resource/IResourceFactory.h"
#include "Resource/IResourceManager.h"
#include "Script/AutoScriptClass.h"
#include "Script/IScriptManager.h"

namespace traktor
{
	namespace parade
	{

void registerResourceClasses(script::IScriptManager* scriptManager)
{
	Ref< script::AutoScriptClass< resource::IResourceFactory > > classIResourceFactory = new script::AutoScriptClass< resource::IResourceFactory >();
	classIResourceFactory->addMethod(L"isCacheable", &resource::IResourceFactory::isCacheable);
	scriptManager->registerClass(classIResourceFactory);

	Ref< script::AutoScriptClass< resource::IResourceHandle > > classIResourceHandle = new script::AutoScriptClass< resource::IResourceHandle >();
	classIResourceHandle->addMethod(L"replace", &resource::IResourceHandle::replace);
	classIResourceHandle->addMethod(L"get", &resource::IResourceHandle::get);
	classIResourceHandle->addMethod(L"flush", &resource::IResourceHandle::flush);
	scriptManager->registerClass(classIResourceHandle);

	Ref< script::AutoScriptClass< resource::IResourceManager > > classIResourceManager = new script::AutoScriptClass< resource::IResourceManager >();
	classIResourceManager->addMethod(L"addFactory", &resource::IResourceManager::addFactory);
	classIResourceManager->addMethod(L"removeFactory", &resource::IResourceManager::removeFactory);
	classIResourceManager->addMethod(L"removeAllFactories", &resource::IResourceManager::removeAllFactories);
	classIResourceManager->addMethod(L"dumpStatistics", &resource::IResourceManager::dumpStatistics);
	scriptManager->registerClass(classIResourceManager);
}

	}
}
