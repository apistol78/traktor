/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/IStream.h"
#include "Core/Io/Reader.h"
#include "Database/Instance.h"
#include "Weather/Clouds/CloudMask.h"
#include "Weather/Clouds/CloudMaskFactory.h"
#include "Weather/Clouds/CloudMaskResource.h"

namespace traktor
{
	namespace weather
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.weather.CloudMaskFactory", CloudMaskFactory, resource::IResourceFactory)

const TypeInfoSet CloudMaskFactory::getResourceTypes() const
{
	return makeTypeInfoSet< CloudMaskResource >();
}

const TypeInfoSet CloudMaskFactory::getProductTypes(const TypeInfo& resourceType) const
{
	return makeTypeInfoSet< CloudMask >();
}

bool CloudMaskFactory::isCacheable(const TypeInfo& productType) const
{
	return true;
}

Ref< Object > CloudMaskFactory::create(resource::IResourceManager* resourceManager, const db::Database* database, const db::Instance* instance, const TypeInfo& productType, const Object* current) const
{
	Ref< const CloudMaskResource > resource = instance->getObject< CloudMaskResource >();
	if (!resource)
		return nullptr;

	Ref< IStream > stream = instance->readData(L"Data");
	if (!stream)
		return nullptr;

	int32_t size = resource->getSize();

	Ref< CloudMask > mask = new CloudMask(size);
	Reader(stream).read(mask->m_data.ptr(), size * size, sizeof(CloudMask::Sample));
	stream->close();

	return mask;
}

	}
}
