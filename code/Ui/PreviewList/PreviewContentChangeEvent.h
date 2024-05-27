/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
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

namespace traktor::ui
{

class PreviewItem;

/*!
 * \ingroup UI
 */
class T_DLLCLASS PreviewContentChangeEvent : public ContentChangeEvent
{
	T_RTTI_CLASS;

public:
	explicit PreviewContentChangeEvent(EventSubject* sender, PreviewItem* item, const std::wstring& originalText);

	PreviewItem* getItem() const;

	const std::wstring& getOriginalText() const;

private:
	Ref< PreviewItem > m_item;
	std::wstring m_originalText;
};

}
