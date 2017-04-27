/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Physics/Editor/PhysicsComponentEditor.h"
#include "Physics/Editor/PhysicsComponentEditorFactory.h"
#include "Physics/World/RigidBodyComponentData.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.PhysicsComponentEditorFactory", PhysicsComponentEditorFactory, scene::IComponentEditorFactory)

const TypeInfoSet PhysicsComponentEditorFactory::getComponentDataTypes() const
{
	return makeTypeInfoSet< RigidBodyComponentData >();
}

Ref< scene::IComponentEditor > PhysicsComponentEditorFactory::createComponentEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter, world::IEntityComponentData* componentData) const
{
	return new PhysicsComponentEditor(context, entityAdapter, componentData);
}

	}
}
