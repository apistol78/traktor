/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "I18N/Text.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Scene/Editor/Events/ModifierChangedEvent.h"
#include "Shape/Editor/Solid/PrimitiveEditModifier.h"
#include "Shape/Editor/Solid/SolidEditorPlugin.h"
#include "Ui/ToolBar/ToolBar.h"
#include "Ui/ToolBar/ToolBarButton.h"

namespace traktor
{
    namespace shape
    {

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.SolidEditorPlugin", SolidEditorPlugin, scene::ISceneEditorPlugin)

SolidEditorPlugin::SolidEditorPlugin(scene::SceneEditorContext* context)
:   m_context(context)
{
}

bool SolidEditorPlugin::create(ui::Widget* parent, ui::ToolBar* toolBar)
{
	// int32_t image = toolBar->addImage(new ui::StyleBitmap(L"Terrain.Terrain"), 15);
    int32_t image = 0;

	auto toolToggleEditPrimitive = new ui::ToolBarButton(i18n::Text(L"SHAPE_EDITOR_EDIT_PRIMITIVE"), image + 0, ui::Command(L"Shape.Editor.EditPrimitive"), ui::ToolBarButton::BsDefaultToggle);
    toolBar->addItem(toolToggleEditPrimitive);

	m_context->addEventHandler< scene::ModifierChangedEvent >(this, &SolidEditorPlugin::eventModifierChanged);
    return true;
}

bool SolidEditorPlugin::handleCommand(const ui::Command& command)
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

void SolidEditorPlugin::eventModifierChanged(scene::ModifierChangedEvent* event)
{
}

    }
}