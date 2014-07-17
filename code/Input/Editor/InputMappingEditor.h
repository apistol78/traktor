#ifndef traktor_input_InputMappingEditor_H
#define traktor_input_InputMappingEditor_H

#include "Editor/IObjectEditor.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace editor
	{

class IEditor;

	}

	namespace ui
	{

class Container;
class ButtonClickEvent;

		namespace custom
		{

class AutoPropertyList;

		}
	}

	namespace input
	{

class InputMappingSourceData;

class T_DLLCLASS InputMappingEditor : public editor::IObjectEditor
{
	T_RTTI_CLASS;

public:
	InputMappingEditor(editor::IEditor* editor);

	virtual bool create(ui::Widget* parent, db::Instance* instance, ISerializable* object);

	virtual void destroy();

	virtual void apply();

	virtual bool handleCommand(const ui::Command& command);

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId);

	virtual ui::Size getPreferredSize() const;

private:
	editor::IEditor* m_editor;
	Ref< db::Instance > m_instance;
	Ref< InputMappingSourceData > m_mapping;
	Ref< ui::Container > m_containerOuter;
	Ref< ui::custom::AutoPropertyList > m_propertyListInput;

	void eventButtonImportInput(ui::ButtonClickEvent* event);

	void eventButtonExportInput(ui::ButtonClickEvent* event);
};

	}
}

#endif	// traktor_input_InputMappingEditor_H
