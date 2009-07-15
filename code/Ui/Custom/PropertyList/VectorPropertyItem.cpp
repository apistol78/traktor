#include <limits>
#include "Ui/Custom/PropertyList/VectorPropertyItem.h"
#include "Ui/Custom/PropertyList/PropertyList.h"
#include "Ui/Edit.h"
#include "Ui/NumericEditValidator.h"
#include "Ui/MethodHandler.h"
#include "Ui/Events/FocusEvent.h"
#include "Ui/Events/MouseEvent.h"
#include "Core/Misc/String.h"

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

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.VectorPropertyItem", VectorPropertyItem, PropertyItem)

VectorPropertyItem::VectorPropertyItem(const std::wstring& text, const vector_t& value, int dimension)
:	PropertyItem(text)
,	m_dimension(dimension)
{
	T_ASSERT (m_dimension > 0);
	T_ASSERT (m_dimension <= MaxDimension);
	std::memcpy(m_value, value, sizeof(m_value));
}

void VectorPropertyItem::setValue(const vector_t& value)
{
	std::memcpy(m_value, value, sizeof(m_value));
}

const VectorPropertyItem::vector_t& VectorPropertyItem::getValue() const
{
	return m_value;
}

void VectorPropertyItem::createInPlaceControls(Widget* parent, bool visible)
{
	for (int i = 0; i < m_dimension; ++i)
	{
		m_editors[i] = gc_new< Edit >();
		m_editors[i]->create(
			parent,
			L"",
			WsNone,
			gc_new< NumericEditValidator >(
				true,
				-std::numeric_limits< float >::max(),
				std::numeric_limits< float >::max()
			)
		);
		m_editors[i]->setVisible(false);
		m_editors[i]->addFocusEventHandler(createMethodHandler(this, &VectorPropertyItem::eventEditFocus));
	}
}

void VectorPropertyItem::destroyInPlaceControls()
{
	for (int i = 0; i < m_dimension; ++i)
		m_editors[i]->destroy();
}

void VectorPropertyItem::resizeInPlaceControls(const Rect& rc, std::vector< WidgetRect >& outChildRects)
{
	for (int i = 0; i < m_dimension; ++i)
	{
		Rect rcSub(
			rc.left + (c_valueWidth * i) / m_dimension,
			rc.top,
			rc.left + (c_valueWidth * (i + 1)) / m_dimension,
			rc.bottom
		);

		outChildRects.push_back(WidgetRect(m_editors[i], rcSub));
	}
}

void VectorPropertyItem::mouseButtonDown(MouseEvent* event)
{
	for (int i = 0; i < m_dimension; ++i)
	{
		Rect rcSub = m_editors[i]->getRect();
		if (rcSub.inside(event->getPosition()))
		{
			m_editors[i]->setText(toString(m_value[i]));
			m_editors[i]->setVisible(true);
			m_editors[i]->setFocus();
			m_editors[i]->selectAll();
			break;
		}
	}
}

void VectorPropertyItem::paintValue(Canvas& canvas, const Rect& rc)
{
	for (int i = 0; i < m_dimension; ++i)
	{
		Rect rcSub(
			rc.left + (c_valueWidth * i) / m_dimension,
			rc.top,
			rc.left + (c_valueWidth * (i + 1)) / m_dimension,
			rc.bottom
		);

		canvas.drawText(
			rcSub.inflate(-2, -2),
			toString(m_value[i]),
			AnLeft,
			AnCenter
		);
	}
}

void VectorPropertyItem::eventEditFocus(Event* event)
{
	FocusEvent* f = static_cast< FocusEvent* >(event);
	if (f->lostFocus())
	{
		for (int i = 0; i < m_dimension; ++i)
		{
			if (m_editors[i]->isVisible(false))
			{
				m_value[i] = parseString< float >(m_editors[i]->getText());
				m_editors[i]->setVisible(false);
			}
		}
		notifyChange();
	}
}

		}
	}
}
