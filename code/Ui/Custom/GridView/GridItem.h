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

	GridItem(const std::wstring& text);

	GridItem(const std::wstring& text, Bitmap* image);

	GridItem(Bitmap* image);

	void setText(const std::wstring& text);

	const std::wstring& getText() const;

	void setImage(Bitmap* image);

	Bitmap* getImage() const;

private:
	std::wstring m_text;
	Ref< Bitmap > m_image;

	virtual int32_t getHeight() const;

	virtual AutoWidgetCell* hitTest(AutoWidget* widget, const Point& position);

	virtual void paint(AutoWidget* widget, Canvas& canvas, const Rect& rect);
};

		}
	}
}

#endif	// traktor_ui_GridItem_H
