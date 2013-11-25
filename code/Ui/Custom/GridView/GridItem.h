#ifndef traktor_ui_GridItem_H
#define traktor_ui_GridItem_H

#include "Ui/Custom/GridView/GridCell.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_CUSTOM_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class Bitmap;
class Font;

		namespace custom
		{

/*! \brief Grid item.
 * \ingroup UIC
 */
class T_DLLCLASS GridItem : public GridCell
{
	T_RTTI_CLASS;

public:
	GridItem();

	explicit GridItem(const std::wstring& text);

	explicit GridItem(const std::wstring& text, Font* font);

	explicit GridItem(const std::wstring& text, Bitmap* image);

	explicit GridItem(Bitmap* image);

	void setText(const std::wstring& text);

	const std::wstring& getText() const;

	void setFont(Font* font);

	Font* getFont() const;

	void setImage(Bitmap* image);

	Bitmap* getImage() const;

private:
	std::wstring m_text;
	Ref< Font > m_font;
	Ref< Bitmap > m_image;

	virtual int32_t getHeight() const;

	virtual AutoWidgetCell* hitTest(AutoWidget* widget, const Point& position);

	virtual void paint(AutoWidget* widget, Canvas& canvas, const Rect& rect);
};

		}
	}
}

#endif	// traktor_ui_GridItem_H
