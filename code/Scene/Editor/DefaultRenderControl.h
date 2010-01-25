#ifndef traktor_scene_DefaultRenderControl_H
#define traktor_scene_DefaultRenderControl_H

#include "Scene/Editor/ISceneRenderControl.h"

namespace traktor
{
	namespace ui
	{

class Event;
class Container;
class Widget;

		namespace custom
		{

class ToolBar;
class ToolBarDropDown;

		}
	}

	namespace scene
	{

class SceneEditorContext;

class DefaultRenderControl : public ISceneRenderControl
{
	T_RTTI_CLASS;

public:
	bool create(ui::Widget* parent, SceneEditorContext* context, int32_t index);

	virtual void destroy();

	virtual void updateWorldRenderer();

	virtual bool handleCommand(const ui::Command& command);

	virtual void update();

private:
	Ref< SceneEditorContext > m_context;
	int32_t m_index;
	Ref< ui::Container > m_container;
	Ref< ui::custom::ToolBar > m_toolBar;
	Ref< ui::custom::ToolBarDropDown > m_toolBarView;
	Ref< ISceneRenderControl > m_renderControl;

	void createRenderControl(int32_t type);

	void eventToolClick(ui::Event* event);
};

	}
}

#endif	// traktor_scene_DefaultRenderControl_H
