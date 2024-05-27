/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/PreviewList/PreviewContentChangeEvent.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.PreviewContentChangeEvent", PreviewContentChangeEvent, ContentChangeEvent)

PreviewContentChangeEvent::PreviewContentChangeEvent(EventSubject* sender, PreviewItem* item, const std::wstring& originalText)
:	ContentChangeEvent(sender)
,	m_item(item)
,	m_originalText(originalText)
{
}

PreviewItem* PreviewContentChangeEvent::getItem() const
{
	return m_item;
}

const std::wstring& PreviewContentChangeEvent::getOriginalText() const
{
	return m_originalText;
}

}
