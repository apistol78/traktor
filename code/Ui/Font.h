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
#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ui
{

/*! Font abstraction.
 * \ingroup UI
 *
 * Font size is expressed in "device independent pixels" (DIP) size which is defined
 * as being pixels on a 1/96 inch DPI device.
 */
class T_DLLCLASS Font : public Object
{
	T_RTTI_CLASS;

public:
	Font();

	Font(const Font& font);

	explicit Font(const std::wstring& face, int32_t size, bool bold = false, bool italic = false, bool underline = false);

	void setFace(const std::wstring& face);

	std::wstring getFace() const;

	void setSize(int32_t size);

	int32_t getSize() const;

	void setPixelSize(int32_t size);

	int32_t getPixelSize() const;

	void setBold(bool bold);

	bool isBold() const;

	void setItalic(bool italic);

	bool isItalic() const;

	void setUnderline(bool underline);

	bool isUnderline() const;

	bool operator == (const Font& r) const;

	bool operator != (const Font& r) const;

	bool operator < (const Font& r) const;

private:
	struct FontParameters
	{
		int32_t size;
		bool bold;
		bool italic;
		bool underline;
	};

	std::wstring m_face;
	FontParameters m_params;
};

}
