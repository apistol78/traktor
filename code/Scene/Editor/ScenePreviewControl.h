#ifndef traktor_scene_ScenePreviewControl_H
#define traktor_scene_ScenePreviewControl_H

#include "Core/RefArray.h"
#include "Core/Timer/Timer.h"
#include "Ui/Container.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

class ToolBar;
class ToolBarButton;
class ToolBarButtonClickEvent;
class ToolBarDropDown;

		}
	}

	namespace scene
	{

class ModifierChangedEvent;
class SceneEditorContext;
class ISceneRenderControl;
class IModifier;

class T_DLLCLASS ScenePreviewControl : public ui::Container
{
	T_RTTI_CLASS;

public:
	ScenePreviewControl();

	bool create(ui::Widget* parent, SceneEditorContext* context);

	void destroy();

	void updateWorldRenderer();

	bool handleCommand(const ui::Command& command);

	bool getViewIndex(const ui::Point& position, uint32_t& outIndex) const;

	virtual ui::Size getPreferedSize() const;

private:
	enum SplitType
	{
		StSingle = 1,
		StDouble = 2,
		StQuadruple = 4
	};

	Ref< ui::EventSubject::IEventHandler > m_idleEventHandler;
	Ref< ui::custom::ToolBar > m_toolBarActions;
	Ref< ui::custom::ToolBarButton > m_toolTogglePick;
	Ref< ui::custom::ToolBarButton > m_toolToggleTranslate;
	Ref< ui::custom::ToolBarButton > m_toolToggleRotate;
	Ref< ui::custom::ToolBarButton > m_toolToggleSnap;
	Ref< ui::custom::ToolBarDropDown > m_toolSnapSpacing;
	int32_t m_splitCount;
	Ref< ui::Widget > m_splitterRenderControls;
	RefArray< ISceneRenderControl > m_renderControls;
	Ref< IModifier > m_modifierTranslate;
	Ref< IModifier > m_modifierRotate;
	Ref< SceneEditorContext > m_context;
	Timer m_timer;
	float m_lastDeltaTime;
	float m_lastPhysicsTime;

	void updateRenderControls();

	void updateEditState();

	void eventToolBarActionClicked(ui::custom::ToolBarButtonClickEvent* event);

	void eventModifierChanged(ModifierChangedEvent* event);

	void eventIdle(ui::IdleEvent* event);
};

	}
}

#endif	// traktor_scene_ScenePreviewControl_H
