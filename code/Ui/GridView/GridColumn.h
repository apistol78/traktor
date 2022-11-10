/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Ui/Associative.h"

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

/*! Grid column.
 * \ingroup UI
 */
class T_DLLCLASS GridColumn
:	public Object
,	public Associative
{
	T_RTTI_CLASS;

public:
	GridColumn(const std::wstring& title, uint32_t width);

	GridColumn(const std::wstring& title, uint32_t width, bool editable);

	void setTitle(const std::wstring& title);

	const std::wstring& getTitle() const;

	void setWidth(uint32_t width);

	uint32_t getWidth() const;

	void setEditable(bool editable);

	bool isEditable() const;

private:
	std::wstring m_title;
	uint32_t m_width;
	bool m_editable;
};

	}
}

