/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Physics/DynamicBodyDesc.h"
#include "Physics/Editor/PhysicsComponentEditor.h"
#include "Physics/Editor/PhysicsComponentEditorFactory.h"
#include "Physics/World/JointComponentData.h"
#include "Physics/World/RigidBodyComponentData.h"
#include "Physics/World/Character/CharacterComponentData.h"
#include "Physics/World/Vehicle/VehicleComponentData.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.PhysicsComponentEditorFactory", PhysicsComponentEditorFactory, scene::IComponentEditorFactory)

const TypeInfoSet PhysicsComponentEditorFactory::getComponentDataTypes() const
{
	TypeInfoSet typeInfoSet;
	typeInfoSet.insert< CharacterComponentData >();
	typeInfoSet.insert< JointComponentData >();
	typeInfoSet.insert< RigidBodyComponentData >();
	typeInfoSet.insert< VehicleComponentData >();
	return typeInfoSet;
}

bool PhysicsComponentEditorFactory::alwaysRebuild(const world::IEntityComponentData* componentData) const
{
	if (is_a< CharacterComponentData >(componentData) || is_a< JointComponentData >(componentData) || is_a< VehicleComponentData >(componentData))
		return true;
	else if (auto rbc = dynamic_type_cast< const RigidBodyComponentData* >(componentData))
	{
		if (is_a< DynamicBodyDesc >(rbc->getBodyDesc()))
			return true;
	}
	return false;
}

Ref< scene::IComponentEditor > PhysicsComponentEditorFactory::createComponentEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter, world::IEntityComponentData* componentData) const
{
	return new PhysicsComponentEditor(context, entityAdapter, componentData);
}

	}
}
