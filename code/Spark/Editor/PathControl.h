/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Spark/Path.h"
#include "Ui/Widget.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace spark
	{

class PathControl : public ui::Widget
{
	T_RTTI_CLASS;

public:
	bool create(ui::Widget* parent, int style);

	void setPath(const Path& path);

private:
	Path m_path;

	Vector2 m_viewOffset;
	float m_viewScale;

	ui::Point m_lastMousePosition;

	std::vector< uint16_t > m_fillStyles;
	int m_fillStyleIndex;

	void eventPaint(ui::PaintEvent* event);

	void eventMouseButtonDown(ui::MouseButtonDownEvent* event);

	void eventMouseButtonUp(ui::MouseButtonUpEvent* event);

	void eventMouseMove(ui::MouseMoveEvent* event);

	void eventMouseWheel(ui::MouseWheelEvent* event);

	void eventKeyDown(ui::KeyDownEvent* event);
};

	}
}

