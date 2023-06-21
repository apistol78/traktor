/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cstring>
#include <limits>
#include "Core/Math/Const.h"
#include "Core/Misc/Split.h"
#include "Core/Misc/String.h"
#include "Ui/Application.h"
#include "Ui/Clipboard.h"
#include "Ui/Edit.h"
#include "Ui/NumericEditValidator.h"
#include "Ui/PropertyList/PropertyList.h"
#include "Ui/PropertyList/AnglesPropertyItem.h"

namespace traktor
{
	namespace ui
	{
		namespace
		{

const Unit c_valueWidth = 200_ut;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.AnglesPropertyItem", AnglesPropertyItem, PropertyItem)

AnglesPropertyItem::AnglesPropertyItem(const std::wstring& text, const Vector4& value)
:	PropertyItem(text)
,	m_value(value)
{
}

void AnglesPropertyItem::setValue(const Vector4& value)
{
	m_value = value;
}

const Vector4& AnglesPropertyItem::getValue() const
{
	return m_value;
}

void AnglesPropertyItem::createInPlaceControls(PropertyList* parent)
{
	for (int i = 0; i < 3; ++i)
	{
		T_ASSERT(!m_editors[i]);
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
		m_editors[i]->addEventHandler< FocusEvent >(this, &AnglesPropertyItem::eventEditFocus);
	}
}

void AnglesPropertyItem::destroyInPlaceControls()
{
	for (int i = 0; i < 3; ++i)
	{
		if (m_editors[i])
		{
			m_editors[i]->destroy();
			m_editors[i] = nullptr;
		}
	}
}

void AnglesPropertyItem::resizeInPlaceControls(const Rect& rc, std::vector< WidgetRect >& outChildRects)
{
	for (int i = 0; i < 3; ++i)
	{
		if (!m_editors[i])
			continue;

		Rect rcSub(
			rc.left + (m_editors[i]->pixel(c_valueWidth) * i) / 3,
			rc.top,
			rc.left + (m_editors[i]->pixel(c_valueWidth) * (i + 1)) / 3,
			rc.bottom
		);

		outChildRects.push_back(WidgetRect(m_editors[i], rcSub));
	}
}

void AnglesPropertyItem::mouseButtonDown(MouseButtonDownEvent* event)
{
	float hpb[4];
	m_value.storeUnaligned(hpb);

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

void AnglesPropertyItem::paintValue(PropertyList* parent, Canvas& canvas, const Rect& rc)
{
	float hpb[4];
	m_value.storeUnaligned(hpb);

	for (int i = 0; i < 3; ++i)
	{
		Rect rcSub(
			rc.left + (parent->pixel(c_valueWidth) * i) / 3,
			rc.top,
			rc.left + (parent->pixel(c_valueWidth) * (i + 1)) / 3,
			rc.bottom
		);

		canvas.drawText(
			rcSub.inflate(-2, 0),
			toString(rad2deg(hpb[i]), 1) + L"\xb0",
			AnLeft,
			AnCenter
		);
	}
}

bool AnglesPropertyItem::copy()
{
	if (isEditing())
		return false;

	Clipboard* clipboard = Application::getInstance()->getClipboard();
	if (!clipboard)
		return false;

	StringOutputStream ss;
	for (int32_t i = 0; i < 3; ++i)
	{
		if (i > 0)
			ss << L",";
		ss << rad2deg(m_value[i]);
	}
	return clipboard->setText(ss.str());
}

bool AnglesPropertyItem::paste()
{
	if (isEditing())
		return false;

	Clipboard* clipboard = Application::getInstance()->getClipboard();
	if (!clipboard)
		return false;

	std::vector< float > values;
	Split< std::wstring, float >::any(clipboard->getText(), L",", values, true);
	if (values.size() >= 3)
	{
		for (int i = 0; i < 3; ++i)
			m_value[i] = Scalar(deg2rad(values[i]));
		return true;
	}
	else
		return false;
}

bool AnglesPropertyItem::isEditing() const
{
	for (int32_t i = 0; i < 3; ++i)
	{
		if (m_editors[i] && m_editors[i]->isVisible(false))
			return true;
	}
	return false;
}

void AnglesPropertyItem::eventEditFocus(FocusEvent* event)
{
	if (event->lostFocus())
	{
		float hpb[4];
		m_value.storeUnaligned(hpb);

		for (int i = 0; i < 3; ++i)
		{
			if (m_editors[i]->isVisible(false))
			{
				hpb[i] = deg2rad(parseString< float >(m_editors[i]->getText()));
				m_editors[i]->setVisible(false);
			}
		}

		m_value = Vector4::loadUnaligned(hpb);

		notifyChange();
	}
}

	}
}
