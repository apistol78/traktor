/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/Widget.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ui
{

class IBitmap;

/*! Static image.
 * \ingroup UI
 */
class T_DLLCLASS Image : public Widget
{
	T_RTTI_CLASS;

public:
	constexpr static uint32_t WsTransparent = WsUser;
	constexpr static uint32_t WsScale = (WsUser << 1);
	constexpr static uint32_t WsScaleKeepAspect = (WsUser << 2);
	constexpr static uint32_t WsNearestFilter = (WsUser << 3);

	bool create(Widget* parent, IBitmap* image = nullptr, uint32_t style = WsNone);

	virtual Size getMinimumSize() const override;

	virtual Size getPreferredSize(const Size& hint) const override;

	virtual Size getMaximumSize() const override;

	bool setImage(IBitmap* image, bool transparent = false);

	IBitmap* getImage() const;

	bool isTransparent() const;

	bool scaling() const;

private:
	Ref< IBitmap > m_image;
	bool m_transparent = false;
	bool m_scale = false;
	bool m_keepAspect = false;
	bool m_nearest = false;

	void eventPaint(PaintEvent* event);
};

}
