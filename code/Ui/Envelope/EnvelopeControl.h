/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "Ui/Widget.h"

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

class EnvelopeEvaluator;
class EnvelopeKey;

/*! Envelope control.
 * \ingroup UI
 */
class T_DLLCLASS EnvelopeControl : public Widget
{
	T_RTTI_CLASS;

public:
	bool create(Widget* parent, EnvelopeEvaluator* evaluator, float minValue = 0.0f, float maxValue = 1.0f, int style = WsNone);

	void insertKey(EnvelopeKey* key);

	const RefArray< EnvelopeKey >& getKeys() const;

	void addRange(const Color4ub& color, float limit0, float limit1, float limit2, float limit3);

private:
	struct Range
	{
		Color4ub color;
		float limits[4];
	};

	Ref< EnvelopeEvaluator > m_evaluator;
	float m_minValue;
	float m_maxValue;
	RefArray< EnvelopeKey > m_keys;
	std::vector< Range > m_ranges;
	Rect m_rcEnv;
	Ref< EnvelopeKey > m_selectedKey;

	void eventButtonDown(MouseButtonDownEvent* event);

	void eventButtonUp(MouseButtonUpEvent* event);

	void eventMouseMove(MouseMoveEvent* event);

	void eventPaint(PaintEvent* event);
};

	}
}

