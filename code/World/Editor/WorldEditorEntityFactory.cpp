/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/Editor/EditorAttributesComponentData.h"
#include "World/Editor/WorldEditorEntityFactory.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.WorldEditorEntityFactory", WorldEditorEntityFactory, AbstractEntityFactory)

const TypeInfoSet WorldEditorEntityFactory::getEntityComponentTypes() const
{
	return makeTypeInfoSet< EditorAttributesComponentData >();
}

Ref< IEntityComponent > WorldEditorEntityFactory::createEntityComponent(const world::IEntityBuilder* builder, const IEntityComponentData& entityComponentData) const
{
	return nullptr;
}

}
