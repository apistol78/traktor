#ifndef traktor_editor_ObjectEditorDialog_H
#define traktor_editor_ObjectEditorDialog_H

#include "Ui/ConfigDialog.h"

namespace traktor
{

class ISerializable;
class PropertyGroup;

	namespace db
	{

class Instance;

	}


	namespace editor
	{

class IEditor;
class IObjectEditor;
class IObjectEditorFactory;
class ObjectEditor;

class ObjectEditorDialog : public ui::ConfigDialog
{
	T_RTTI_CLASS;

public:
	ObjectEditorDialog(PropertyGroup* settings, const IObjectEditorFactory* objectEditorFactory);

	bool create(IEditor* editor, ui::Widget* parent, db::Instance* instance, ISerializable* object);

	void destroy();

	bool apply(bool keep);

	void cancel();

	bool handleCommand(const ui::Command& command);

	void handleDatabaseEvent(db::Database* database, const Guid& eventId);

private:
	Ref< PropertyGroup > m_settings;
	Ref< const IObjectEditorFactory > m_objectEditorFactory;
	Ref< IObjectEditor > m_objectEditor;
	Ref< ObjectEditor > m_editor;
	Ref< db::Instance > m_instance;
	uint32_t m_objectHash;
	bool m_modified;

	void eventClick(ui::Event* event);

	void eventClose(ui::Event* event);

	void eventTimer(ui::Event* event);
};

	}
}

#endif	// traktor_editor_ObjectEditorDialog_H
