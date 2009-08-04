#ifndef traktor_physics_SprayEditorProfile_H
#define traktor_physics_SprayEditorProfile_H

#include "Scene/Editor/ISceneEditorProfile.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace physics
	{

class T_DLLCLASS PhysicsEditorProfile : public scene::ISceneEditorProfile
{
	T_RTTI_CLASS(PhysicsEditorProfile)

public:
	virtual TypeSet getEntityDataTypes() const;

	virtual void getCommands(
		std::list< ui::Command >& outCommands
	) const;

	virtual void createToolBarItems(
		ui::custom::ToolBar* toolBar
	) const;

	virtual void createResourceFactories(
		scene::SceneEditorContext* context,
		RefArray< resource::IResourceFactory >& outResourceFactories
	) const;

	virtual void createEntityFactories(
		scene::SceneEditorContext* context,
		RefArray< world::IEntityFactory >& outEntityFactories
	) const;

	virtual void createEntityRenderers(
		scene::SceneEditorContext* context,
		render::IRenderView* renderView,
		render::PrimitiveRenderer* primitiveRenderer,
		RefArray< world::IEntityRenderer >& outEntityRenderers
	) const;

	virtual scene::IEntityEditor* createEntityEditor(
		scene::SceneEditorContext* context,
		const Type& entityDataType
	) const;
};

	}
}

#endif	// traktor_physics_SprayEditorProfile_H
