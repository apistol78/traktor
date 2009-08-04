#ifndef traktor_scene_DefaultEditorProfile_H
#define traktor_scene_DefaultEditorProfile_H

#include "Scene/Editor/ISceneEditorProfile.h"

namespace traktor
{
	namespace scene
	{

/*! \brief Default scene editor profile. */
class DefaultEditorProfile : public ISceneEditorProfile
{
	T_RTTI_CLASS(DefaultEditorProfile)

public:
	virtual TypeSet getEntityDataTypes() const;

	virtual void getCommands(
		std::list< ui::Command >& outCommands
	) const;

	virtual void createToolBarItems(
		ui::custom::ToolBar* toolBar
	) const;

	virtual void createResourceFactories(
		SceneEditorContext* context,
		RefArray< resource::IResourceFactory >& outResourceFactories
	) const;

	virtual void createEntityFactories(
		SceneEditorContext* context,
		RefArray< world::IEntityFactory >& outEntityFactories
	) const;

	virtual void createEntityRenderers(
		SceneEditorContext* context,
		render::IRenderView* renderView,
		render::PrimitiveRenderer* primitiveRenderer,
		RefArray< world::IEntityRenderer >& outEntityRenderers
	) const;

	virtual IEntityEditor* createEntityEditor(
		SceneEditorContext* context,
		const Type& entityDataType
	) const;
};

	}
}

#endif	// traktor_scene_DefaultEditorProfile_H
