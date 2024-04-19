/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Database/Instance.h"
#include "Render/Resource/AliasTextureResource.h"
#include "Render/Resource/AliasTextureFactory.h"
#include "Render/Resource/TextureProxy.h"
#include "Resource/IResourceManager.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.AliasTextureFactory", 0, AliasTextureFactory, resource::IResourceFactory)

bool AliasTextureFactory::initialize(const ObjectStore& objectStore)
{
	return true;
}

const TypeInfoSet AliasTextureFactory::getResourceTypes() const
{
	return makeTypeInfoSet< AliasTextureResource >();
}

const TypeInfoSet AliasTextureFactory::getProductTypes(const TypeInfo& resourceType) const
{
	return makeTypeInfoSet< ITexture >();
}

bool AliasTextureFactory::isCacheable(const TypeInfo& productType) const
{
	return true;
}

Ref< Object > AliasTextureFactory::create(resource::IResourceManager* resourceManager, const db::Database* database, const db::Instance* instance, const TypeInfo& productType, const Object* current) const
{
	Ref< const AliasTextureResource > resource = instance->getObject< const AliasTextureResource >();
	if (!resource)
		return nullptr;

	resource::Proxy< ITexture > texture;
	if (!resourceManager->bind(resource->m_texture, texture))
		return nullptr;

	return new TextureProxy(texture);
}

}
