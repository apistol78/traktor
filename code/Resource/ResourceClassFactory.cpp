/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/Boxes/BoxedGuid.h"
#include "Core/Class/Boxes/BoxedTypeInfo.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Resource/FileBundle.h"
#include "Resource/IResourceFactory.h"
#include "Resource/IResourceManager.h"
#include "Resource/ResourceBundle.h"
#include "Resource/ResourceClassFactory.h"

namespace traktor::resource
{
	namespace
	{

Ref< ResourceHandle > IResourceManager_bind(IResourceManager* self, const TypeInfo& type, const Any& id)
{
	if (CastAny< Guid >::accept(id))
		return self->bind(type, CastAny< Guid >::get(id));
	else if (id.isString())
		return self->bind(type, Guid(id.getWideString()));
	else
		return nullptr;
}

void IResourceManager_reload(IResourceManager* self, const Any& guidOrType, bool flushedOnly)
{
	if (CastAny< Guid >::accept(guidOrType))
		self->reload(CastAny< Guid >::get(guidOrType), flushedOnly);
	else if (CastAny< TypeInfo >::accept(guidOrType))
		self->reload(CastAny< TypeInfo >::get(guidOrType), flushedOnly);
	else if (guidOrType.isString())
		self->reload(Guid(guidOrType.getWideString()), flushedOnly);
}

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.resource.ResourceClassFactory", 0, ResourceClassFactory, IRuntimeClassFactory)

void ResourceClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	auto classFileBundle = new AutoRuntimeClass< FileBundle >();
	classFileBundle->addMethod("lookup", &FileBundle::lookup);
	registrar->registerClass(classFileBundle);

	auto classResourceBundle = new AutoRuntimeClass< ResourceBundle >();
	registrar->registerClass(classResourceBundle);

	auto classIResourceFactory = new AutoRuntimeClass< IResourceFactory >();
	classIResourceFactory->addMethod("isCacheable", &IResourceFactory::isCacheable);
	registrar->registerClass(classIResourceFactory);

	auto classIResourceHandle = new AutoRuntimeClass< ResourceHandle >();
	classIResourceHandle->addMethod("replace", &ResourceHandle::replace);
	classIResourceHandle->addMethod("get", &ResourceHandle::get);
	classIResourceHandle->addMethod("flush", &ResourceHandle::flush);
	registrar->registerClass(classIResourceHandle);

	auto classIResourceManager = new AutoRuntimeClass< IResourceManager >();
	classIResourceManager->addMethod("addFactory", &IResourceManager::addFactory);
	classIResourceManager->addMethod("removeFactory", &IResourceManager::removeFactory);
	classIResourceManager->addMethod("removeAllFactories", &IResourceManager::removeAllFactories);
	classIResourceManager->addMethod("load", &IResourceManager::load);
	classIResourceManager->addMethod("bind", &IResourceManager_bind);
	classIResourceManager->addMethod("reload", &IResourceManager_reload);
	classIResourceManager->addMethod("unload", &IResourceManager::unload);
	classIResourceManager->addMethod("unloadUnusedResident", &IResourceManager::unloadUnusedResident);
	registrar->registerClass(classIResourceManager);
}

}
