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

class Font;
class IBitmap;

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

	explicit GridItem(const std::wstring& text, IBitmap* image);

	explicit GridItem(IBitmap* image);

	virtual void setText(const std::wstring& text) T_OVERRIDE T_FINAL;

	virtual std::wstring getText() const T_OVERRIDE T_FINAL;

	virtual bool edit() T_OVERRIDE;

	void setFont(Font* font);

	Font* getFont() const;

	void setImage(IBitmap* image);

	IBitmap* getImage() const;

private:
	std::wstring m_text;
	Ref< Font > m_font;
	Ref< IBitmap > m_image;

	virtual int32_t getHeight() const T_OVERRIDE T_FINAL;

	virtual AutoWidgetCell* hitTest(const Point& position) T_OVERRIDE T_FINAL;

	virtual void paint(Canvas& canvas, const Rect& rect) T_OVERRIDE T_FINAL;
};

		}
	}
}

#endif	// traktor_ui_GridItem_H
