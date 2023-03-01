/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/Application.h"
#include "Ui/Edit.h"
#include "Ui/Static.h"
#include "Ui/TableLayout.h"
#include "Ui/DropDown.h"
#include "Ui/InputDialog.h"
#include "Ui/MiniButton.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.InputDialog", InputDialog, ConfigDialog)

bool InputDialog::create(
	Widget* parent,
	const std::wstring& title,
	const std::wstring& message,
	Field* outFields,
	uint32_t outFieldsCount
)
{
	if (!ConfigDialog::create(parent, title, dpi96(1280), dpi96(720), ConfigDialog::WsDefaultFixed, new TableLayout(L"100%", L"*,*", 4, 4)))
		return false;

	Ref< Static > labelMessage = new Static();
	labelMessage->create(this, message);

	Ref< Container > container = new Container();
	container->create(this, WsNone, new TableLayout(L"*,*", L"*", 0, 4));

	m_outFields = outFields;
	for (uint32_t i = 0; i < outFieldsCount; ++i)
	{
		Ref< Static > labelEdit = new Static();
		labelEdit->create(container, m_outFields[i].title);

		if (!m_outFields[i].valueEnumerator)
		{
			if (!m_outFields[i].browseFile)
			{
				Ref< Edit > edit = new Edit();
				edit->create(container, m_outFields[i].value, WsTabStop, m_outFields[i].validator);
				m_editFields.push_back(edit);
			}
			else
			{
				Ref< Container > fieldContainer = new Container();
				fieldContainer->create(container, WsNone, new TableLayout(L"100%,*", L"*", 0, 4));

				Ref< Edit > edit = new Edit();
				edit->create(fieldContainer, m_outFields[i].value, WsTabStop, m_outFields[i].validator);
				m_editFields.push_back(edit);

				Ref< MiniButton > browse = new MiniButton();
				browse->create(fieldContainer, L"...");
			}
		}
		else
		{
			Ref< DropDown > dropDown = new DropDown();
			dropDown->create(container, WsTabStop);

			std::wstring key, value;
			for (size_t index = 0; m_outFields[i].valueEnumerator->getValue(index, key, value); ++index)
				dropDown->add(key);

			dropDown->select(0);
			m_editFields.push_back(dropDown);
		}
	}

	fit(Container::Both);

	return true;
}

DialogResult InputDialog::showModal()
{
	if (m_editFields.empty())
		return DialogResult::Cancel;

	Ref< Edit > edit = dynamic_type_cast< Edit* >(m_editFields.front());
	if (edit)
	{
		edit->setFocus();
		edit->selectAll();
	}

	const DialogResult result = ConfigDialog::showModal();
	if (result == DialogResult::Ok && m_outFields)
	{
		for (uint32_t i = 0; i < uint32_t(m_editFields.size()); ++i)
		{
			Ref< Edit > editField = dynamic_type_cast< Edit* >(m_editFields[i]);
			if (editField)
				m_outFields[i].value = editField->getText();

			Ref< DropDown > dropDown = dynamic_type_cast< DropDown* >(m_editFields[i]);
			if (dropDown)
			{
				m_outFields[i].selectedIndex = dropDown->getSelected();
				if (m_outFields[i].selectedIndex >= 0)
				{
					std::wstring key, value;
					if (m_outFields[i].valueEnumerator->getValue(
						(size_t)m_outFields[i].selectedIndex,
						key,
						value
					))
						m_outFields[i].value = value;
				}
			}
		}
	}

	return result;
}

}
