/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_physics_RigidEntityEditor_H
#define traktor_physics_RigidEntityEditor_H

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

class T_DLLCLASS RigidEntityEditor : public scene::DefaultEntityEditor
{
	T_RTTI_CLASS;

public:
	RigidEntityEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter);

	virtual void entitySelected(bool selected) override final;

	virtual bool handleCommand(const ui::Command& command) override final;

	virtual void drawGuide(render::PrimitiveRenderer* primitiveRenderer) const override final;

private:
	PhysicsRenderer m_physicsRenderer;
	bool m_showHull;
};

	}
}

#endif	// traktor_physics_RigidEntityEditor_H
