#ifndef traktor_ui_custom_PreviewItem_H
#define traktor_ui_custom_PreviewItem_H

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

class Bitmap;

		namespace custom
		{

class T_DLLCLASS PreviewItem : public AutoWidgetCell
{
	T_RTTI_CLASS;

public:
	PreviewItem();

	PreviewItem(const std::wstring& text);

	void setText(const std::wstring& text);

	const std::wstring& getText() const;

	void setImage(ui::Bitmap* image);

	ui::Bitmap* getImage() const;

	void setSelected(bool selected);

	bool isSelected() const;

	virtual void paint(Canvas& canvas, const Rect& rect) T_OVERRIDE;

private:
	std::wstring m_text;
	Ref< ui::Bitmap > m_bitmapImage;
	bool m_selected;
};

		}
	}
}

#endif	// traktor_ui_custom_PreviewItem_H
