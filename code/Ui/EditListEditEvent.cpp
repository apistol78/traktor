/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/EditListEditEvent.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.EditListEditEvent", EditListEditEvent, ContentChangeEvent)

EditListEditEvent::EditListEditEvent(EventSubject* sender, Edit* edit, int32_t index, const std::wstring& text)
:	ContentChangeEvent(sender)
,	m_edit(edit)
,	m_index(index)
,	m_text(text)
{
}

Edit* EditListEditEvent::getEdit() const
{
	return m_edit;
}

int32_t EditListEditEvent::getIndex() const
{
	return m_index;
}

const std::wstring& EditListEditEvent::getText() const
{
	return m_text;
}

}
