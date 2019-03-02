#pragma once

#include "Ui/Auto/AutoWidgetCell.h"

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

class Bitmap;

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

	virtual void paint(Canvas& canvas, const Rect& rect) override;

private:
	std::wstring m_text;
	Ref< ui::Bitmap > m_bitmapImage;
	bool m_selected;
};

	}
}

