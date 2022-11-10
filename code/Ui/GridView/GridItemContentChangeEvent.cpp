/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/GridView/GridItemContentChangeEvent.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.GridItemContentChangeEvent", GridItemContentChangeEvent, ContentChangeEvent)

GridItemContentChangeEvent::GridItemContentChangeEvent(EventSubject* sender, GridItem* item, const std::wstring& originalText)
:	ContentChangeEvent(sender)
,	m_item(item)
,	m_originalText(originalText)
{
}

GridItem* GridItemContentChangeEvent::getItem() const
{
	return m_item;
}

const std::wstring& GridItemContentChangeEvent::getOriginalText() const
{
	return m_originalText;
}

	}
}
