#pragma once

#include "Ui/ConfigDialog.h"

namespace traktor
{

class ISerializable;

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
	explicit ObjectEditorDialog(const IObjectEditorFactory* objectEditorFactory);

	bool create(IEditor* editor, ui::Widget* parent, db::Instance* instance, ISerializable* object);

	virtual void destroy() override final;

	bool apply(bool keep);

	void cancel();

	bool handleCommand(const ui::Command& command);

	void handleDatabaseEvent(db::Database* database, const Guid& eventId);

private:
	Ref< const IObjectEditorFactory > m_objectEditorFactory;
	Ref< IObjectEditor > m_objectEditor;
	Ref< ObjectEditor > m_editor;
	Ref< db::Instance > m_instance;
	Guid m_instanceGuid;
	uint32_t m_objectHash;
	bool m_modified;

	void eventClick(ui::ButtonClickEvent* event);

	void eventClose(ui::CloseEvent* event);

	void eventTimer(ui::TimerEvent* event);
};

	}
}

