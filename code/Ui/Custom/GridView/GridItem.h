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

	virtual void setText(const std::wstring& text) T_FINAL;

	virtual std::wstring getText() const T_FINAL;

	virtual bool edit();

	void setFont(Font* font);

	Font* getFont() const;

	void setImage(Bitmap* image);

	Bitmap* getImage() const;

private:
	std::wstring m_text;
	Ref< Font > m_font;
	Ref< Bitmap > m_image;

	virtual int32_t getHeight() const T_FINAL;

	virtual AutoWidgetCell* hitTest(const Point& position) T_FINAL;

	virtual void paint(Canvas& canvas, const Rect& rect) T_FINAL;
};

		}
	}
}

#endif	// traktor_ui_GridItem_H
