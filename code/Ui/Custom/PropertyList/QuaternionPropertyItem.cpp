#include <cstring>
#include <limits>
#include "Core/Math/Const.h"
#include "Core/Misc/String.h"
#include "Ui/Edit.h"
#include "Ui/MethodHandler.h"
#include "Ui/NumericEditValidator.h"
#include "Ui/Events/FocusEvent.h"
#include "Ui/Events/MouseEvent.h"
#include "Ui/Custom/PropertyList/PropertyList.h"
#include "Ui/Custom/PropertyList/QuaternionPropertyItem.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{
			namespace
			{

const int c_valueWidth = 200;

			}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.QuaternionPropertyItem", QuaternionPropertyItem, PropertyItem)

QuaternionPropertyItem::QuaternionPropertyItem(const std::wstring& text, const Quaternion& value)
:	PropertyItem(text)
,	m_value(value)
{
}

void QuaternionPropertyItem::setValue(const Quaternion& value)
{
	m_value = value;
}

const Quaternion& QuaternionPropertyItem::getValue() const
{
	return m_value;
}

void QuaternionPropertyItem::createInPlaceControls(Widget* parent)
{
	for (int i = 0; i < 3; ++i)
	{
		T_ASSERT (!m_editors[i]);
		m_editors[i] = new Edit();
		m_editors[i]->create(
			parent,
			L"",
			WsNone,
			new NumericEditValidator(
				true,
				-std::numeric_limits< float >::max(),
				std::numeric_limits< float >::max(),
				1
			)
		);
		m_editors[i]->setVisible(false);
		m_editors[i]->addFocusEventHandler(createMethodHandler(this, &QuaternionPropertyItem::eventEditFocus));
	}
}

void QuaternionPropertyItem::destroyInPlaceControls()
{
	for (int i = 0; i < 3; ++i)
	{
		if (m_editors[i])
		{
			m_editors[i]->destroy();
			m_editors[i] = 0;
		}
	}
}

void QuaternionPropertyItem::resizeInPlaceControls(const Rect& rc, std::vector< WidgetRect >& outChildRects)
{
	for (int i = 0; i < 3; ++i)
	{
		if (!m_editors[i])
			continue;

		Rect rcSub(
			rc.left + (c_valueWidth * i) / 3,
			rc.top,
			rc.left + (c_valueWidth * (i + 1)) / 3,
			rc.bottom
		);

		outChildRects.push_back(WidgetRect(m_editors[i], rcSub));
	}
}

void QuaternionPropertyItem::mouseButtonDown(MouseEvent* event)
{
	float hpb[3];
	m_value.toEulerAngles(hpb[0], hpb[1], hpb[2]);

	for (int i = 0; i < 3; ++i)
	{
		Rect rcSub = m_editors[i]->getRect();
		if (rcSub.inside(event->getPosition()))
		{
			m_editors[i]->setText(toString(rad2deg(hpb[i]), 1));
			m_editors[i]->setVisible(true);
			m_editors[i]->setFocus();
			m_editors[i]->selectAll();
			break;
		}
	}
}

void QuaternionPropertyItem::paintValue(Canvas& canvas, const Rect& rc)
{
	float hpb[3];
	m_value.toEulerAngles(hpb[0], hpb[1], hpb[2]);

	for (int i = 0; i < 3; ++i)
	{
		Rect rcSub(
			rc.left + (c_valueWidth * i) / 3,
			rc.top,
			rc.left + (c_valueWidth * (i + 1)) / 3,
			rc.bottom
		);

		canvas.drawText(
			rcSub.inflate(-2, -2),
			toString(rad2deg(hpb[i]), 1) + L"\xb0",
			AnLeft,
			AnCenter
		);
	}
}

void QuaternionPropertyItem::eventEditFocus(Event* event)
{
	FocusEvent* f = static_cast< FocusEvent* >(event);
	if (f->lostFocus())
	{
		float hpb[3];
		m_value.toEulerAngles(hpb[0], hpb[1], hpb[2]);

		for (int i = 0; i < 3; ++i)
		{
			if (m_editors[i]->isVisible(false))
			{
				hpb[i] = deg2rad(parseString< float >(m_editors[i]->getText()));
				m_editors[i]->setVisible(false);
			}
		}

		m_value = Quaternion(hpb[0], hpb[1], hpb[2]);

		notifyChange();
	}
}

		}
	}
}
