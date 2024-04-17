/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Theater/TheaterComponent.h"
#include "Theater/TheaterComponentData.h"
#include "Theater/TheaterEntityFactory.h"

namespace traktor::theater
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.theater.TheaterEntityFactory", TheaterEntityFactory, world::AbstractEntityFactory)

const TypeInfoSet TheaterEntityFactory::getWorldComponentTypes() const
{
	return makeTypeInfoSet< TheaterComponentData >();
}

Ref< world::IWorldComponent > TheaterEntityFactory::createWorldComponent(const world::IEntityBuilder* builder, const world::IWorldComponentData& worldComponentData) const
{
	if (auto theaterComponentData = dynamic_type_cast< const TheaterComponentData* >(&worldComponentData))
		return theaterComponentData->createInstance(false);
	else
		return nullptr;
}

}
