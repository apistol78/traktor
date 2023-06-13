/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Math/Color4f.h"
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

/*! Color property item.
 * \ingroup UI
 */
class T_DLLCLASS ColorPropertyItem : public PropertyItem
{
	T_RTTI_CLASS;

public:
	ColorPropertyItem(const std::wstring& text, const Color4f& value, bool hdr);

	void setValue(const Color4f& value);

	const Color4f& getValue() const;

	bool getHighDynamicRange() const;

protected:
	virtual void mouseButtonUp(MouseButtonUpEvent* event) override final;

	virtual void paintValue(PropertyList* parent, Canvas& canvas, const Rect& rc) override final;

	virtual bool copy() override final;

	virtual bool paste() override final;

private:
	Color4f m_value;
	bool m_hdr;
	Rect m_rcColor;
};

	}
}
