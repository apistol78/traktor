/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_sound_GrainProperties_H
#define traktor_sound_GrainProperties_H

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

#endif	// traktor_sound_GrainProperties_H
