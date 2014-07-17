#include "Ui/Bitmap.h"
#include "Ui/Custom/PropertyList/CheckPropertyItem.h"
#include "Ui/Custom/PropertyList/PropertyList.h"

// Resources
#include "Resources/Unchecked.h"
#include "Resources/Checked.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{
			namespace
			{

Ref< Bitmap > s_imageUnchecked;
Ref< Bitmap > s_imageChecked;

			}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.CheckProperyItem", CheckPropertyItem, PropertyItem)

CheckPropertyItem::CheckPropertyItem(const std::wstring& text, bool checked)
:	PropertyItem(text)
,	m_checked(checked)
{
	if (!s_imageUnchecked)
		s_imageUnchecked = Bitmap::load(c_ResourceUnchecked, sizeof(c_ResourceUnchecked), L"png");
	if (!s_imageChecked)
		s_imageChecked = Bitmap::load(c_ResourceChecked, sizeof(c_ResourceChecked), L"png");
}

void CheckPropertyItem::setChecked(bool checked)
{
	m_checked = checked;
}

bool CheckPropertyItem::isChecked() const
{
	return m_checked;
}

void CheckPropertyItem::mouseButtonDown(MouseButtonDownEvent* event)
{
	m_checked = !m_checked;
	notifyChange();
}

void CheckPropertyItem::paintValue(Canvas& canvas, const Rect& rc)
{
	Bitmap* image = m_checked ? s_imageChecked : s_imageUnchecked;
	T_ASSERT (image);

	int c = (rc.getHeight() - image->getSize().cy) / 2;
	canvas.drawBitmap(
		ui::Point(rc.left + 2, rc.top + c),
		ui::Point(0, 0),
		image->getSize(),
		image
	);
}

		}
	}
}
