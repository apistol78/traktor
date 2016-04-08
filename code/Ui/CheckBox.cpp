#include "Ui/Application.h"
#include "Ui/Canvas.h"
#include "Ui/CheckBox.h"
#include "Ui/StyleBitmap.h"
#include "Ui/StyleSheet.h"

// Resources
#include "Resources/Unchecked.h"
#include "Resources/Checked.h"

namespace traktor
{
	namespace ui
	{
			namespace
			{

Ref< IBitmap > s_imageUnchecked;
Ref< IBitmap > s_imageChecked;

			}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.CheckBox", CheckBox, Widget)

CheckBox::CheckBox()
:	m_checked(false)
{
	if (!s_imageUnchecked)
		s_imageUnchecked = new StyleBitmap(L"UI.Unchecked", c_ResourceUnchecked, sizeof(c_ResourceUnchecked));
	if (!s_imageChecked)
		s_imageChecked = new StyleBitmap(L"UI.Checked", c_ResourceChecked, sizeof(c_ResourceChecked));
}

bool CheckBox::create(Widget* parent, const std::wstring& text, bool checked)
{
	if (!Widget::create(parent))
		return false;

	setText(text);
	setChecked(checked);

	addEventHandler< PaintEvent >(this, &CheckBox::eventPaint);
	addEventHandler< MouseButtonDownEvent >(this, &CheckBox::eventButtonDown);
	addEventHandler< MouseButtonUpEvent >(this, &CheckBox::eventButtonUp);

	return true;
}

void CheckBox::setChecked(bool checked)
{
	m_checked = checked;
}

bool CheckBox::isChecked() const
{
	return m_checked;
}

Size CheckBox::getPreferedSize() const
{
	Size sz = getTextExtent(getText());
	sz.cx += s_imageUnchecked->getSize().cx + scaleBySystemDPI(4);
	sz.cy = std::max(sz.cy, s_imageUnchecked->getSize().cy);
	return sz;
}

void CheckBox::eventPaint(PaintEvent* event)
{
	const StyleSheet* ss = Application::getInstance()->getStyleSheet();
	Canvas& canvas = event->getCanvas();
	
	Rect rcInner = getInnerRect();
	
	canvas.setBackground(ss->getColor(this, L"background-color"));
	canvas.fillRect(rcInner);

	IBitmap* image = m_checked ? s_imageChecked : s_imageUnchecked;
	T_ASSERT (image);

	int32_t y = (rcInner.getHeight() - image->getSize().cy) / 2;

	canvas.drawBitmap(
		Point(0, y),
		Point(0, 0),
		image->getSize(),
		image,
		BmAlpha
	);

	Rect rcText = rcInner;
	rcText.left += image->getSize().cx + scaleBySystemDPI(4);

	canvas.setForeground(ss->getColor(this, L"color"));
	canvas.drawText(rcText, getText(), AnLeft, AnCenter);

	event->consume();
}

void CheckBox::eventButtonDown(MouseButtonDownEvent* event)
{
	setFocus();
	setCapture();
}

void CheckBox::eventButtonUp(MouseButtonUpEvent* event)
{
	if (!hasCapture())
		return;

	releaseCapture();

	if (getInnerRect().inside(event->getPosition()))
	{
		m_checked = !m_checked;
		update();

		ButtonClickEvent clickEvent(this);
		raiseEvent(&clickEvent);
	}
}

	}
}
