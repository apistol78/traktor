/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Ui/PropertyList/AutoPropertyList.h"

namespace traktor
{
	namespace editor
	{

class IEditor;

	}

	namespace ui
	{

class Command;
class HierarchicalState;
class PropertyCommandEvent;
class PropertyContentChangeEvent;

	}

	namespace sound
	{

class IGrainData;

class GrainProperties
:	public ui::EventSubject
,	public ui::PropertyList::IPropertyGuidResolver
{
	T_RTTI_CLASS;

public:
	GrainProperties(editor::IEditor* editor);

	bool create(ui::Widget* parent);

	void destroy();

	void set(IGrainData* grain);

	void reset();

	bool handleCommand(const ui::Command& command);

private:
	editor::IEditor* m_editor;
	Ref< ui::AutoPropertyList > m_propertyList;
	Ref< IGrainData > m_grain;
	std::map< const TypeInfo*, Ref< ui::HierarchicalState > > m_states;

	virtual bool resolvePropertyGuid(const Guid& guid, std::wstring& resolved) const override final;

	void eventPropertyCommand(ui::PropertyCommandEvent* event);

	void eventPropertyChange(ui::PropertyContentChangeEvent* event);
};

	}
}

