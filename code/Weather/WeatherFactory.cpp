/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Weather/WeatherFactory.h"
#include "Weather/Clouds/CloudComponent.h"
#include "Weather/Clouds/CloudComponentData.h"
#include "Weather/Fog/VolumetricFogComponent.h"
#include "Weather/Fog/VolumetricFogComponentData.h"
#include "Weather/Precipitation/PrecipitationComponent.h"
#include "Weather/Precipitation/PrecipitationComponentData.h"
#include "Weather/Sky/SkyComponent.h"
#include "Weather/Sky/SkyComponentData.h"

namespace traktor::weather
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.weather.WeatherFactory", WeatherFactory, world::IEntityFactory)

WeatherFactory::WeatherFactory(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem)
:	m_resourceManager(resourceManager)
,	m_renderSystem(renderSystem)
{
}

const TypeInfoSet WeatherFactory::getEntityTypes() const
{
	return TypeInfoSet();
}

const TypeInfoSet WeatherFactory::getEntityEventTypes() const
{
	return TypeInfoSet();
}

const TypeInfoSet WeatherFactory::getEntityComponentTypes() const
{
	return makeTypeInfoSet<
		CloudComponentData,
		PrecipitationComponentData,
		SkyComponentData,
		VolumetricFogComponentData
	>();
}

Ref< world::Entity > WeatherFactory::createEntity(const world::IEntityBuilder* builder, const world::EntityData& entityData) const
{
	return nullptr;
}

Ref< world::IEntityEvent > WeatherFactory::createEntityEvent(const world::IEntityBuilder* builder, const world::IEntityEventData& entityEventData) const
{
	return nullptr;
}

Ref< world::IEntityComponent > WeatherFactory::createEntityComponent(const world::IEntityBuilder* builder, const world::IEntityComponentData& entityComponentData) const
{
	if (const CloudComponentData* cloudComponentData = dynamic_type_cast< const CloudComponentData* >(&entityComponentData))
		return cloudComponentData->createComponent(m_resourceManager, m_renderSystem);
	else if (const PrecipitationComponentData* precipitationComponentData = dynamic_type_cast< const PrecipitationComponentData* >(&entityComponentData))
		return precipitationComponentData->createComponent(m_resourceManager);
	else if (const SkyComponentData* skyComponentData = dynamic_type_cast< const SkyComponentData* >(&entityComponentData))
		return skyComponentData->createComponent(m_resourceManager, m_renderSystem);
	else if (const VolumetricFogComponentData* volumetricFogComponentData = dynamic_type_cast< const VolumetricFogComponentData* >(&entityComponentData))
		return volumetricFogComponentData->createComponent(m_resourceManager, m_renderSystem);
	else
		return nullptr;
}

}
