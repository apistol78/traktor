#ifndef traktor_sound_BankControlGrain_H
#define traktor_sound_BankControlGrain_H

#include "Core/Ref.h"
#include "Ui/Bitmap.h"
#include "Ui/Custom/Auto/AutoWidgetCell.h"

namespace traktor
{
	namespace sound
	{

class IGrainData;

class BankControlGrain : public ui::custom::AutoWidgetCell
{
	T_RTTI_CLASS;

public:
	BankControlGrain(BankControlGrain* parent, IGrainData* grain, const std::wstring& text, int32_t image);

	BankControlGrain* getParent() const;

	IGrainData* getGrain() const;

	int32_t getImage() const;

	std::wstring getText() const;

	virtual void mouseDown(ui::custom::AutoWidget* widget, const ui::Point& position);

	virtual void paint(ui::custom::AutoWidget* widget, ui::Canvas& canvas, const ui::Rect& rect);

private:
	Ref< ui::Bitmap > m_bitmapGrain;
	BankControlGrain* m_parent;
	Ref< IGrainData > m_grain;
	std::wstring m_text;
	int32_t m_image;
};

	}
}

#endif	// traktor_sound_BankControlGrain_H
