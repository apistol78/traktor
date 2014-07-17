#ifndef traktor_scene_EntityDependencyInvestigator_H
#define traktor_scene_EntityDependencyInvestigator_H

#include "Ui/Container.h"

namespace traktor
{
	namespace ui
	{

class TreeView;

	}

	namespace scene
	{

class SceneEditorContext;
class EntityAdapter;

/*! \brief Entity asset dependency investigator. */
class EntityDependencyInvestigator : public ui::Container
{
	T_RTTI_CLASS;

public:
	EntityDependencyInvestigator(SceneEditorContext* context);

	virtual void destroy();

	bool create(ui::Widget* parent);

	void setEntityAdapter(EntityAdapter* entityAdapter);

private:
	Ref< SceneEditorContext > m_context;
	Ref< ui::TreeView > m_dependencyTree;
	Ref< EntityAdapter > m_currentEntityAdapter;

	void eventDependencyActivate(ui::TreeViewItemActivateEvent* event);

	void eventContextSelect(ui::SelectionChangeEvent* event);
};

	}
}

#endif	// traktor_scene_EntityDependencyInvestigator_H
