/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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
class RedrawEvent;
class SceneEditorContext;
class ISceneRenderControl;
class IModifier;

class T_DLLCLASS ScenePreviewControl : public ui::Container
{
	T_RTTI_CLASS;

public:
	ScenePreviewControl();

	bool create(ui::Widget* parent, SceneEditorContext* context);

	virtual void destroy() T_OVERRIDE T_FINAL;

	void updateWorldRenderer();

	bool handleCommand(const ui::Command& command);

	bool getViewIndex(const ui::Point& position, uint32_t& outIndex) const;

	virtual ui::Size getPreferedSize() const T_OVERRIDE T_FINAL;

private:
	enum SplitType
	{
		StSingle = 1,
		StDouble = 2,
		StQuadruple = 4
	};

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
	float m_lastTime;
	float m_lastPhysicsTime;

	bool updateRenderControls();

	void updateEditState();

	void eventToolBarActionClicked(ui::custom::ToolBarButtonClickEvent* event);

	void eventModifierChanged(ModifierChangedEvent* event);

	void eventRedraw(RedrawEvent* event);

	void eventTimer(ui::TimerEvent* event);
};

	}
}

#endif	// traktor_scene_ScenePreviewControl_H
