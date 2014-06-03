#ifndef traktor_sound_GrainProperties_H
#define traktor_sound_GrainProperties_H

#include "Core/Object.h"
#include "Ui/Custom/PropertyList/AutoPropertyList.h"

namespace traktor
{
	namespace editor
	{

class IEditor;

	}

	namespace ui
	{

class Command;
class Event;
class HierarchicalState;

	}

	namespace sound
	{

class IGrainData;

class GrainProperties
:	public ui::EventSubject
,	public ui::custom::PropertyList::IPropertyGuidResolver
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
	Ref< ui::custom::AutoPropertyList > m_grainPropertyList;
	Ref< IGrainData > m_grain;
	std::map< const TypeInfo*, Ref< ui::HierarchicalState > > m_states;

	virtual bool resolvePropertyGuid(const Guid& guid, std::wstring& resolved) const;

	void eventPropertyCommand(ui::Event* event);

	void eventPropertyChange(ui::Event* event);
};

	}
}

#endif	// traktor_sound_GrainProperties_H
