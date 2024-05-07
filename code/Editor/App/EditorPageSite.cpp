/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Editor/App/DefaultPropertiesView.h"
#include "Editor/App/EditorPageSite.h"
#include "Editor/App/EditorForm.h"

namespace traktor::editor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.EditorPageSite", EditorPageSite, IEditorPageSite)

EditorPageSite::EditorPageSite(EditorForm* editor, bool siteVisible)
:	m_editor(editor)
,	m_siteVisible(siteVisible)
{
}

void EditorPageSite::show()
{
	if (m_siteVisible)
		return;

	for (auto& panelWidget : m_panelWidgets)
	{
		if (panelWidget.second)
			m_editor->showAdditionalPanel(panelWidget.first);
		else
			m_editor->hideAdditionalPanel(panelWidget.first);
	}

	m_siteVisible = true;
}

void EditorPageSite::hide()
{
	if (!m_siteVisible)
		return;

	for (auto& panelWidget : m_panelWidgets)
	{
		panelWidget.second = m_editor->isAdditionalPanelVisible(panelWidget.first);
		m_editor->hideAdditionalPanel(panelWidget.first);
	}

	m_siteVisible = false;
}

Ref< PropertiesView > EditorPageSite::createPropertiesView(ui::Widget* parent)
{
	Ref< DefaultPropertiesView > propertiesView = new DefaultPropertiesView(m_editor);
	if (propertiesView->create(parent))
		return propertiesView;
	else
		return nullptr;
}

void EditorPageSite::createAdditionalPanel(ui::Widget* widget, ui::Unit size, bool south)
{
	m_panelWidgets.push_back({ widget, true });
	m_editor->createAdditionalPanel(widget, size, south ? 0 : 1);
}

void EditorPageSite::destroyAdditionalPanel(ui::Widget* widget)
{
	auto it = std::find_if(m_panelWidgets.begin(), m_panelWidgets.end(), [&](const std::pair< ui::Widget*, bool >& v) { return v.first == widget; });
	T_FATAL_ASSERT(it != m_panelWidgets.end());

	m_panelWidgets.erase(it);
	m_editor->destroyAdditionalPanel(widget);
}

void EditorPageSite::showAdditionalPanel(ui::Widget* widget)
{
	auto it = std::find_if(m_panelWidgets.begin(), m_panelWidgets.end(), [&](const std::pair< ui::Widget*, bool >& v) { return v.first == widget; });
	T_FATAL_ASSERT(it != m_panelWidgets.end());

	if (m_siteVisible)
		m_editor->showAdditionalPanel(widget);
	else
		it->second = true;
}

void EditorPageSite::hideAdditionalPanel(ui::Widget* widget)
{
	auto it = std::find_if(m_panelWidgets.begin(), m_panelWidgets.end(), [&](const std::pair< ui::Widget*, bool >& v) { return v.first == widget; });
	T_FATAL_ASSERT(it != m_panelWidgets.end());

	if (m_siteVisible)
		m_editor->hideAdditionalPanel(widget);
	else
		it->second = false;
}

bool EditorPageSite::isAdditionalPanelVisible(const ui::Widget* widget) const
{
	auto it = std::find_if(m_panelWidgets.begin(), m_panelWidgets.end(), [&](const std::pair< ui::Widget*, bool >& v) { return v.first == widget; });
	T_FATAL_ASSERT(it != m_panelWidgets.end());

	if (m_siteVisible)
		return m_editor->isAdditionalPanelVisible(widget);
	else
		return it->second;
}

}
