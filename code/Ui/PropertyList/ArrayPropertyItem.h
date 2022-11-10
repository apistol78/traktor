/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

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

class MiniButton;

/*! Array property item.
 * \ingroup UI
 */
class T_DLLCLASS ArrayPropertyItem : public PropertyItem
{
	T_RTTI_CLASS;

public:
	ArrayPropertyItem(const std::wstring& text, const TypeInfo* elementType, bool readOnly);

	void setElementType(const TypeInfo* objectType);

	const TypeInfo* getElementType() const;

protected:
	virtual bool needRemoveChildButton() const override;

	virtual void createInPlaceControls(PropertyList* parent) override;

	virtual void destroyInPlaceControls() override;

	virtual void resizeInPlaceControls(const Rect& rc, std::vector< WidgetRect >& outChildRects) override;

	virtual void paintValue(Canvas& canvas, const Rect& rc) override;

	virtual bool paste() override;

private:
	const TypeInfo* m_elementType;
	bool m_readOnly;
	Ref< MiniButton > m_buttonEdit;

	void eventClick(ButtonClickEvent* event);
};

	}
}

