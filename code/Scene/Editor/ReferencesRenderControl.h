#ifndef traktor_scene_ReferencesRenderControl_H
#define traktor_scene_ReferencesRenderControl_H

#include "Scene/Editor/ISceneRenderControl.h"

namespace traktor
{
	namespace ui
	{

class Widget;
class Container;

		namespace custom
		{

class GraphControl;

		}
	}

	namespace scene
	{

class SceneEditorContext;

class ReferencesRenderControl : public ISceneRenderControl
{
	T_RTTI_CLASS;

public:
	ReferencesRenderControl();

	bool create(ui::Widget* parent, SceneEditorContext* context);

	void destroy();

	virtual void updateWorldRenderer();

	virtual void setAspect(float aspect);

	virtual bool handleCommand(const ui::Command& command);

	virtual void update();

private:
	Ref< ui::Container > m_container;
	Ref< ui::custom::GraphControl > m_referenceGraph;
};

	}
}

#endif	// traktor_scene_ReferencesRenderControl_H
