/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/Point.h"
#include "Ui/PropertyList/PropertyItem.h"

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

class Edit;
class IBitmap;

/*! Numeric property item.
 * \ingroup UI
 */
class T_DLLCLASS NumericPropertyItem : public PropertyItem
{
	T_RTTI_CLASS;

public:
	enum Representation
	{
		RpNormal,
		RpDecibel,
		RpAngle,
		RpAnglesPerSecond,
		RpMetres,
		RpMetresPerSecond,
		RpKilograms,
		RpPercent,
		RpHerz,
		RpEV
	};

	NumericPropertyItem(const std::wstring& text, double value, double limitMin, double limitMax, bool floatPoint, bool hex, Representation representation);

	void setValue(double value);

	double getValue() const;

	void setLimitMin(double limitMin);

	double getLimitMin() const;

	void setLimitMax(double limitMax);

	double getLimitMax() const;

	void setLimit(double limitMin, double limitMax);

protected:
	virtual void createInPlaceControls(PropertyList* parent) override;

	virtual void destroyInPlaceControls() override;

	virtual void resizeInPlaceControls(const Rect& rc, std::vector< WidgetRect >& outChildRects) override;

	virtual void mouseButtonDown(MouseButtonDownEvent* event) override;

	virtual void mouseButtonUp(MouseButtonUpEvent* event) override;

	virtual void mouseMove(MouseMoveEvent* event) override;

	virtual void paintValue(Canvas& canvas, const Rect& rc) override;

	virtual bool copy() override;

	virtual bool paste() override;

private:
	Ref< Edit > m_editor;
	Ref< IBitmap > m_upDown[4];
	double m_value;
	double m_limitMin;
	double m_limitMax;
	bool m_floatPoint;
	bool m_hex;
	Representation m_representation;
	bool m_mouseAdjust;
	Point m_mouseLastPosition;

	void eventEditFocus(FocusEvent* event);

	void eventEditKeyDownEvent(KeyDownEvent* event);
};

	}
}

