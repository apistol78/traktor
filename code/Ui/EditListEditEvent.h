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

class Edit;

/*! Edit event.
 * \ingroup UI
 */
class T_DLLCLASS EditListEditEvent : public ContentChangeEvent
{
	T_RTTI_CLASS;

public:
	EditListEditEvent(EventSubject* sender, Edit* edit, int32_t index, const std::wstring& text);

	Edit* getEdit() const;

	int32_t getIndex() const;

	const std::wstring& getText() const;

private:
	Ref< Edit > m_edit;
	int32_t m_index;
	std::wstring m_text;
};

	}
}

