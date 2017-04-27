/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_Font_H
#define traktor_ui_Font_H

#include <string>
#include "Core/Object.h"

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

/*! \brief Font
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

	Font(const std::wstring& face, int32_t size, bool bold = false, bool italic = false, bool underline = false);

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
		int size;
		bool bold;
		bool italic;
		bool underline;
	};

	std::wstring m_face;
	FontParameters m_params;
};

	}
}

#endif	// traktor_ui_Font_H
