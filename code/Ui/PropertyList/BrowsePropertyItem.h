/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Guid.h"
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

/*! Browse property item.
 * \ingroup UI
 */
class T_DLLCLASS BrowsePropertyItem : public PropertyItem
{
	T_RTTI_CLASS;

public:
	BrowsePropertyItem(const std::wstring& text, const TypeInfo* filterType, const Guid& value);

	void setFilterType(const TypeInfo* filterType);

	const TypeInfo* getFilterType() const;

	void setValue(const Guid& text);

	const Guid& getValue() const;

protected:
	virtual void createInPlaceControls(PropertyList* parent) override;

	virtual void destroyInPlaceControls() override;

	virtual void resizeInPlaceControls(const Rect& rc, std::vector< WidgetRect >& outChildRects) override;

	virtual void paintValue(PropertyList* parent, Canvas& canvas, const Rect& rc) override;

	virtual bool copy() override;

	virtual bool paste() override;

private:
	const TypeInfo* m_filterType;
	Guid m_value;
	Ref< MiniButton > m_buttonEdit;
	Ref< MiniButton > m_buttonBrowse;

	void eventEditClick(ButtonClickEvent* event);

	void eventBrowseClick(ButtonClickEvent* event);
};

	}
}

