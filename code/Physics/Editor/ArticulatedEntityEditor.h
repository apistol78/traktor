/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_physics_ArticulatedEntityEditor_H
#define traktor_physics_ArticulatedEntityEditor_H

#include "Physics/Editor/PhysicsRenderer.h"
#include "Scene/Editor/DefaultEntityEditor.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace physics
	{

class T_DLLCLASS ArticulatedEntityEditor : public scene::DefaultEntityEditor
{
	T_RTTI_CLASS;

public:
	ArticulatedEntityEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter);

	virtual void drawGuide(render::PrimitiveRenderer* primitiveRenderer) const T_OVERRIDE T_FINAL;

private:
	PhysicsRenderer m_physicsRenderer;
};

	}
}

#endif	// traktor_physics_ArticulatedEntityEditor_H
