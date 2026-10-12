/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Scene/Editor/BrowseEntityDialog.h"

#include "Core/Misc/String.h"
#include "I18N/Text.h"
#include "Ui/Edit.h"
#include "Ui/Events/ContentChangeEvent.h"
#include "Ui/Events/MouseDoubleClickEvent.h"
#include "Ui/ListBox/ListBox.h"
#include "Ui/StyleBitmap.h"
#include "Ui/TableLayout.h"

namespace traktor::scene
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.BrowseEntityDialog", BrowseEntityDialog, ui::ConfigDialog)

bool BrowseEntityDialog::create(ui::Widget* parent, const AlignedVector< std::pair< std::wstring, Guid > >& entities, const Guid& current)
{
	if (!ui::ConfigDialog::create(parent, i18n::Text(L"SCENE_EDITOR_BROWSE_ENTITY"), 600_ut, 600_ut, ui::ConfigDialog::WsCenterParent | ui::ConfigDialog::WsDefaultResizable, new ui::TableLayout(L"100%", L"*,100%", 4_ut, 4_ut)))
		return false;

	setIcon(new ui::StyleBitmap(L"Editor.Icon"));

	m_entities = entities;
	m_current = current;

	m_filter = new ui::Edit();
	m_filter->create(this, L"");
	m_filter->addEventHandler< ui::ContentChangeEvent >(this, &BrowseEntityDialog::eventFilterChange);

	m_list = new ui::ListBox();
	m_list->create(this, ui::ListBox::WsSingle);
	m_list->addEventHandler< ui::MouseDoubleClickEvent >(this, &BrowseEntityDialog::eventDoubleClick);

	fill();
	update();
	return true;
}

Guid BrowseEntityDialog::getSelected() const
{
	const int32_t selected = m_list->getSelected();
	return (selected >= 0 && selected < (int32_t)m_shown.size()) ? m_shown[selected] : Guid();
}

void BrowseEntityDialog::fill()
{
	const std::wstring filter = toLower(m_filter->getText());

	m_list->removeAll();
	m_shown.clear();

	for (const auto& entity : m_entities)
	{
		if (!filter.empty() && toLower(entity.first).find(filter) == std::wstring::npos)
			continue;
		m_list->add(entity.first);
		m_shown.push_back(entity.second);
		if (entity.second == m_current)
			m_list->select((int32_t)m_shown.size() - 1);
	}

	m_list->update();
}

void BrowseEntityDialog::eventFilterChange(ui::ContentChangeEvent* event)
{
	fill();
}

void BrowseEntityDialog::eventDoubleClick(ui::MouseDoubleClickEvent* event)
{
	if (m_list->getSelected() >= 0)
		endModal(ui::DialogResult::Ok);
}

}
