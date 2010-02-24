#ifndef traktor_sound_DefaultGrainEditor_H
#define traktor_sound_DefaultGrainEditor_H

#include "Sound/Editor/Resound/IGrainEditor.h"
#include "Ui/Custom/PropertyList/AutoPropertyList.h"

namespace traktor
{
	namespace editor
	{

class IEditor;

	}

	namespace ui
	{

class Event;

	}

	namespace sound
	{

class DefaultGrainEditor
:	public IGrainEditor
,	public ui::custom::PropertyList::IPropertyGuidResolver
{
	T_RTTI_CLASS;

public:
	DefaultGrainEditor(editor::IEditor* editor);

	virtual bool create(ui::Widget* parent);

	virtual void destroy();

	virtual void show(IGrain* grain);

	virtual void hide();

private:
	editor::IEditor* m_editor;
	Ref< ui::custom::AutoPropertyList > m_grainPropertyList;

	virtual bool resolvePropertyGuid(const Guid& guid, std::wstring& resolved) const;

	void eventPropertyCommand(ui::Event* event);
};

	}
}

#endif	// traktor_sound_DefaultGrainEditor_H
