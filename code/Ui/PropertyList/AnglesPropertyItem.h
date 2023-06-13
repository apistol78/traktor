/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Math/Vector4.h"
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

/*! Angles property item.
 * \ingroup UI
 */
class T_DLLCLASS AnglesPropertyItem : public PropertyItem
{
	T_RTTI_CLASS;

public:
	AnglesPropertyItem(const std::wstring& text, const Vector4& value);

	void setValue(const Vector4& value);

	const Vector4& getValue() const;

protected:
	virtual void createInPlaceControls(PropertyList* parent) override;

	virtual void destroyInPlaceControls() override;

	virtual void resizeInPlaceControls(const Rect& rc, std::vector< WidgetRect >& outChildRects) override;

	virtual void mouseButtonDown(MouseButtonDownEvent* event) override;

	virtual void paintValue(PropertyList* parent, Canvas& canvas, const Rect& rc) override;

	virtual bool copy() override;

	virtual bool paste() override;

private:
	Ref< Edit > m_editors[3];
	Vector4 m_value;

	bool isEditing() const;

	void eventEditFocus(FocusEvent* event);
};

	}
}

