/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Compute/ComputeTextureFactory.h"

#include "Core/Misc/ObjectStore.h"
#include "Database/Instance.h"
#include "Render/IRenderSystem.h"
#include "Render/Shader.h"
#include "Render/Compute/ComputeTexture.h"
#include "Render/Compute/ComputeTextureResource.h"
#include "Resource/IResourceManager.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ComputeTextureFactory", 0, ComputeTextureFactory, resource::IResourceFactory)

ComputeTextureFactory::ComputeTextureFactory(IRenderSystem* renderSystem)
:	m_renderSystem(renderSystem)
{
}

bool ComputeTextureFactory::initialize(const ObjectStore& objectStore)
{
	m_renderSystem = objectStore.get< IRenderSystem >();
	return true;
}

const TypeInfoSet ComputeTextureFactory::getResourceTypes() const
{
	return makeTypeInfoSet< ComputeTextureResource >();
}

const TypeInfoSet ComputeTextureFactory::getProductTypes(const TypeInfo& resourceType) const
{
	return makeTypeInfoSet< ITexture >();
}

bool ComputeTextureFactory::isCacheable(const TypeInfo& productType) const
{
	return true;
}

Ref< Object > ComputeTextureFactory::create(resource::IResourceManager* resourceManager, const db::Database* database, const db::Instance* instance, const TypeInfo& productType, const Object* current) const
{
	Ref< const ComputeTextureResource > resource = instance->getObject< ComputeTextureResource >();
	if (!resource)
		return nullptr;

	resource::Proxy< Shader > shader;
	if (!resourceManager->bind(resource->getShader(), shader))
		return nullptr;

	Ref< ComputeTexture > computeTexture = new ComputeTexture();
	if (!computeTexture->create(
		m_renderSystem,
		shader,
		resource->getWidth(),
		resource->getHeight(),
		resource->getFormat(),
		resource->isContinuous()
	))
		return nullptr;

	return computeTexture;
}

void ComputeTextureFactory::destroy(Object* resource) const
{
}

}
