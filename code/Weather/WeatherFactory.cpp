/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Weather/WeatherFactory.h"
#include "Weather/Precipitation/PrecipitationComponent.h"
#include "Weather/Precipitation/PrecipitationComponentData.h"
#include "Weather/Sky/SkyComponent.h"
#include "Weather/Sky/SkyComponentData.h"

namespace traktor::weather
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.weather.WeatherFactory", WeatherFactory, world::AbstractEntityFactory)

WeatherFactory::WeatherFactory(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem)
:	m_resourceManager(resourceManager)
,	m_renderSystem(renderSystem)
{
}

const TypeInfoSet WeatherFactory::getEntityComponentTypes() const
{
	return makeTypeInfoSet<
		PrecipitationComponentData,
		SkyComponentData
	>();
}

Ref< world::IEntityComponent > WeatherFactory::createEntityComponent(const world::IEntityBuilder* builder, const world::IEntityComponentData& entityComponentData) const
{
	if (const PrecipitationComponentData* precipitationComponentData = dynamic_type_cast< const PrecipitationComponentData* >(&entityComponentData))
		return precipitationComponentData->createComponent(m_resourceManager);
	else if (const SkyComponentData* skyComponentData = dynamic_type_cast< const SkyComponentData* >(&entityComponentData))
		return skyComponentData->createComponent(m_resourceManager, m_renderSystem);
	else
		return nullptr;
}

}
