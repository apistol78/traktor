/*
 * TRAKTOR
 * Copyright (c) 2024-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Theater/TheaterEntityFactory.h"

#include "Theater/TheaterEntityComponent.h"
#include "Theater/TheaterEntityComponentData.h"
#include "Theater/TheaterWorldComponent.h"
#include "Theater/TheaterWorldComponentData.h"

namespace traktor::theater
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.theater.TheaterEntityFactory", 0, TheaterEntityFactory, world::AbstractEntityFactory)

const TypeInfoSet TheaterEntityFactory::getEntityComponentTypes() const
{
	return makeTypeInfoSet< TheaterEntityComponentData >();
}

const TypeInfoSet TheaterEntityFactory::getWorldComponentTypes() const
{
	return makeTypeInfoSet< TheaterWorldComponentData >();
}

Ref< world::IEntityComponent > TheaterEntityFactory::createEntityComponent(const world::IEntityBuilder* builder, const world::IEntityComponentData& entityComponentData) const
{
	if (auto theaterEntityComponentData = dynamic_type_cast< const TheaterEntityComponentData* >(&entityComponentData))
		return theaterEntityComponentData->createComponent(builder);
	else
		return nullptr;
}

Ref< world::IWorldComponent > TheaterEntityFactory::createWorldComponent(const world::IEntityBuilder* builder, const world::IWorldComponentData& worldComponentData) const
{
	if (auto theaterWorldComponentData = dynamic_type_cast< const TheaterWorldComponentData* >(&worldComponentData))
		return theaterWorldComponentData->createInstance(builder, false);
	else
		return nullptr;
}

}
