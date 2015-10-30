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
 */
class T_DLLCLASS Font : public Object
{
	T_RTTI_CLASS;

public:
	Font();

	Font(const Font& font);

	Font(const std::wstring& face, int size, bool bold = false, bool italic = false, bool underline = false);

	void setFace(const std::wstring& face);

	std::wstring getFace() const;

	void setSize(int size);

	int getSize() const;

	int getPointSize() const;

	int getPixelSize() const;

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
