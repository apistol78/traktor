/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/ObjectStore.h"
#include "Render/IRenderSystem.h"
#include "Resource/IResourceManager.h"
#include "Weather/WeatherFactory.h"
#include "Weather/Precipitation/PrecipitationComponent.h"
#include "Weather/Precipitation/PrecipitationComponentData.h"
#include "Weather/Sky/SkyComponent.h"
#include "Weather/Sky/SkyComponentData.h"

namespace traktor::weather
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.weather.WeatherFactory", 0, WeatherFactory, world::AbstractEntityFactory)

bool WeatherFactory::initialize(const ObjectStore& objectStore)
{
	m_resourceManager = objectStore.get< resource::IResourceManager >();
	m_renderSystem = objectStore.get< render::IRenderSystem >();
	return true;
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
