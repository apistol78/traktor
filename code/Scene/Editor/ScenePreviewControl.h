/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "Core/Timer/Timer.h"
#include "Ui/Container.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class ToolBar;
class ToolBarButton;
class ToolBarButtonClickEvent;
class ToolBarDropDown;

	}

	namespace scene
	{

class ModifierChangedEvent;
class RedrawEvent;
class SceneEditorContext;
class ISceneRenderControl;
class IModifier;

/*! Scene preview UI control.
 * \ingroup Scene
 */
class T_DLLCLASS ScenePreviewControl : public ui::Container
{
	T_RTTI_CLASS;

public:
	ScenePreviewControl();

	bool create(ui::Widget* parent, SceneEditorContext* context);

	virtual void destroy() override final;

	bool handleCommand(const ui::Command& command);

	bool getViewIndex(const ui::Point& position, uint32_t& outIndex) const;

	virtual ui::Size getPreferredSize(const ui::Size& hint) const override final;

private:
	enum SplitType
	{
		StSingle = 1,
		StDouble = 2,
		StQuadruple = 4
	};

	Ref< ui::ToolBar > m_toolBarActions;
	Ref< ui::ToolBarButton > m_toolTogglePick;
	Ref< ui::ToolBarButton > m_toolToggleTranslate;
	Ref< ui::ToolBarButton > m_toolToggleRotate;
	Ref< ui::ToolBarButton > m_toolToggleScale;
	Ref< ui::ToolBarButton > m_toolToggleSnap;
	Ref< ui::ToolBarDropDown > m_toolSnapSpacing;
	int32_t m_splitCount;
	Ref< ui::Widget > m_splitterRenderControls;
	RefArray< ISceneRenderControl > m_renderControls;
	Ref< IModifier > m_modifierTranslate;
	Ref< IModifier > m_modifierRotate;
	Ref< IModifier > m_modifierScale;
	Ref< SceneEditorContext > m_context;
	Timer m_timer;
	float m_lastTime;
	float m_lastPhysicsTime;

	bool updateRenderControls();

	void updateEditState();

	void eventToolBarActionClicked(ui::ToolBarButtonClickEvent* event);

	void eventModifierChanged(ModifierChangedEvent* event);

	void eventRedraw(RedrawEvent* event);

	void eventTimer(ui::TimerEvent* event);
};

	}
}

