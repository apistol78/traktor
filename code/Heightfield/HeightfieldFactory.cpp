/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/IStream.h"
#include "Database/Instance.h"
#include "Heightfield/Heightfield.h"
#include "Heightfield/HeightfieldFactory.h"
#include "Heightfield/HeightfieldFormat.h"
#include "Heightfield/HeightfieldResource.h"

namespace traktor::hf
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.hf.HeightfieldFactory", 0, HeightfieldFactory, resource::IResourceFactory)

bool HeightfieldFactory::initialize(const ObjectStore& objectStore)
{
	return true;
}

const TypeInfoSet HeightfieldFactory::getResourceTypes() const
{
	return makeTypeInfoSet< HeightfieldResource >();
}

const TypeInfoSet HeightfieldFactory::getProductTypes(const TypeInfo& resourceType) const
{
	return makeTypeInfoSet< Heightfield >();
}

bool HeightfieldFactory::isCacheable(const TypeInfo& productType) const
{
	return true;
}

Ref< Object > HeightfieldFactory::create(resource::IResourceManager* resourceManager, const db::Database* database, const db::Instance* instance, const TypeInfo& productType, const Object* current) const
{
	Ref< const HeightfieldResource > resource = instance->getObject< HeightfieldResource >();
	if (!resource)
		return nullptr;

	Ref< IStream > stream = instance->readData(L"Data");
	if (!stream)
		return nullptr;

	Ref< Heightfield > heightfield = HeightfieldFormat().read(stream, resource->getWorldExtent());

	stream->close();
	return heightfield;
}

}
