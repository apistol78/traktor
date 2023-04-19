/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Ui/Point.h"
#include "Ui/Events/AllEvents.h"

namespace traktor::ui
{

class Widget;

}

namespace traktor::scene
{

class ISceneRenderControl;
class SceneEditorContext;
class TransformChain;

/*! Common model for 3d render control editors. */
class RenderControlModel : public Object
{
	T_RTTI_CLASS;

public:
	RenderControlModel();

	void update(ISceneRenderControl* renderControl, ui::Widget* renderWidget, SceneEditorContext* context, const TransformChain& transformChain);

	void eventButtonDown(ISceneRenderControl* renderControl, ui::Widget* renderWidget, ui::MouseButtonDownEvent* event, SceneEditorContext* context, const TransformChain& transformChain);

	void eventButtonUp(ISceneRenderControl* renderControl, ui::Widget* renderWidget, ui::MouseButtonUpEvent* event, SceneEditorContext* context, const TransformChain& transformChain);

	void eventDoubleClick(ISceneRenderControl* renderControl, ui::Widget* renderWidget, ui::MouseDoubleClickEvent* event, SceneEditorContext* context, const TransformChain& transformChain);

	void eventMouseMove(ISceneRenderControl* renderControl, ui::Widget* renderWidget, ui::MouseMoveEvent* event, SceneEditorContext* context, const TransformChain& transformChain);

	void eventKeyDown(ISceneRenderControl* renderControl, ui::Widget* renderWidget, ui::KeyDownEvent* event, SceneEditorContext* context, const TransformChain& transformChain);

	void eventKeyUp(ISceneRenderControl* renderControl, ui::Widget* renderWidget, ui::KeyUpEvent* event, SceneEditorContext* context, const TransformChain& transformChain);

private:
	enum ModifyType
	{
		MtNothing,
		MtModifier,
		MtCamera,
		MtSelection
	};

	ui::Point m_mousePosition0;
	ui::Point m_mousePosition;
	int32_t m_mouseButton;
	bool m_modifyBegun;
	bool m_modifyAlternative;
	bool m_modifyClone;
	ModifyType m_modify;
	uint32_t m_moveCamera;
};

}
