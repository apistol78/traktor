#ifndef traktor_ui_GridItem_H
#define traktor_ui_GridItem_H

#include "Ui/Custom/Auto/AutoWidgetCell.h"

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

class Font;
class IBitmap;

		namespace custom
		{

class GridRow;

/*! \brief Grid item.
 * \ingroup UIC
 */
class T_DLLCLASS GridItem : public AutoWidgetCell
{
	T_RTTI_CLASS;

public:
	GridItem();

	explicit GridItem(const std::wstring& text);

	explicit GridItem(const std::wstring& text, Font* font);

	explicit GridItem(const std::wstring& text, IBitmap* image);

	explicit GridItem(IBitmap* image);

	void setText(const std::wstring& text);

	std::wstring getText() const;

	bool edit();

	void setFont(Font* font);

	Font* getFont() const;

	void setImage(IBitmap* image);

	IBitmap* getImage() const;

	int32_t getHeight() const;

	GridRow* getRow() const;

private:
	friend class GridRow;

	GridRow* m_row;
	std::wstring m_text;
	Ref< Font > m_font;
	Ref< IBitmap > m_image;

	virtual AutoWidgetCell* hitTest(const Point& position) T_OVERRIDE T_FINAL;

	virtual void paint(Canvas& canvas, const Rect& rect) T_OVERRIDE T_FINAL;
};

		}
	}
}

#endif	// traktor_ui_GridItem_H
