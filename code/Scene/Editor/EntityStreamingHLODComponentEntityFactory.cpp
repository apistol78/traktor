/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Scene/Editor/EntityStreamingHLODComponentEntityFactory.h"

#include "Scene/Editor/EntityStreamingHLODComponentData.h"
#include "World/IEntityComponent.h"

namespace traktor::scene
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.EntityStreamingHLODComponentEntityFactory", EntityStreamingHLODComponentEntityFactory, world::AbstractEntityFactory)

const TypeInfoSet EntityStreamingHLODComponentEntityFactory::getEntityComponentTypes() const
{
	return makeTypeInfoSet< EntityStreamingHLODComponentData >();
}

Ref< world::IEntityComponent > EntityStreamingHLODComponentEntityFactory::createEntityComponent(const world::IEntityBuilder* builder, const world::IEntityComponentData& entityComponentData) const
{
	return nullptr;
}

}
