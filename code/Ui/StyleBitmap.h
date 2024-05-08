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
#include "Ui/IBitmap.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ui
{

/*! Styled bitmap.
 * 
 * Styled bitmaps are automatically resolved
 * through stylesheet when referenced.
 * 
 * \ingroup UI
 */
class T_DLLCLASS StyleBitmap : public IBitmap
{
	T_RTTI_CLASS;

public:
	explicit StyleBitmap(const wchar_t* const name, int32_t index = -1);

	virtual ~StyleBitmap();

	virtual void destroy() override final;

	virtual Size getSize(const Widget* reference) const override final;

	virtual Ref< drawing::Image > getImage(const Widget* reference) const override final;

	virtual ISystemBitmap* getSystemBitmap(const Widget* reference) const override final;

private:
	const wchar_t* const m_name;
	int32_t m_index = -1;
	mutable ISystemBitmap* m_bitmap = nullptr;
	mutable std::wstring m_fileName;
	mutable int32_t m_dpi = -1;

	bool resolve(const Widget* reference, int32_t dpi) const;
};

}
