/*
 * TRAKTOR
 * Copyright (c) 2022 -2026Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Shape/Editor/Solid/SolidSceneEditorUIExtension.h"

#include "I18N/Text.h"
#include "Scene/Editor/Events/ModifierChangedEvent.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Shape/Editor/Solid/PrimitiveEditModifier.h"
#include "Ui/ToolBar/ToolBar.h"
#include "Ui/ToolBar/ToolBarButton.h"

namespace traktor::shape
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.SolidSceneEditorUIExtension", SolidSceneEditorUIExtension, scene::ISceneEditorUIExtension)

SolidSceneEditorUIExtension::SolidSceneEditorUIExtension(scene::SceneEditorContext* context)
	: m_context(context)
{
}

bool SolidSceneEditorUIExtension::create(ui::Widget* parent, ui::ToolBar* toolBar)
{
	// int32_t image = toolBar->addImage(new ui::StyleBitmap(L"Terrain.Terrain"), 15);
	int32_t image = 0;

	auto toolToggleEditPrimitive = new ui::ToolBarButton(i18n::Text(L"SHAPE_EDITOR_EDIT_PRIMITIVE"), image + 0, ui::Command(L"Shape.Editor.EditPrimitive"), ui::ToolBarButton::BsDefaultToggle);
	toolBar->addItem(toolToggleEditPrimitive);

	m_context->addEventHandler< scene::ModifierChangedEvent >(this, &SolidSceneEditorUIExtension::eventModifierChanged);
	return true;
}

bool SolidSceneEditorUIExtension::handleCommand(const ui::Command& command)
{
	if (command == L"Shape.Editor.EditPrimitive")
	{
		m_context->setPlaying(false);
		m_context->setModifier(new PrimitiveEditModifier(m_context));
		return true;
	}
	else
		return false;
}

void SolidSceneEditorUIExtension::eventModifierChanged(scene::ModifierChangedEvent* event)
{
}

}
