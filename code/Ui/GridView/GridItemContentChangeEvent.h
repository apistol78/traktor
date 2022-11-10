/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Ui/Events/ContentChangeEvent.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class GridItem;

/*! \Event when content of a grid view item changes.
 * \ingroup UI
 */
class T_DLLCLASS GridItemContentChangeEvent : public ContentChangeEvent
{
	T_RTTI_CLASS;

public:
	GridItemContentChangeEvent(EventSubject* sender, GridItem* item, const std::wstring& originalText);

	GridItem* getItem() const;

	const std::wstring& getOriginalText() const;

private:
	Ref< GridItem > m_item;
	std::wstring m_originalText;
};

	}
}

