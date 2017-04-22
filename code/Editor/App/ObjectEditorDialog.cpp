#include "Core/Io/StringOutputStream.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Database/Instance.h"
#include "Editor/IObjectEditor.h"
#include "Editor/IObjectEditorFactory.h"
#include "Editor/App/ObjectEditor.h"
#include "Editor/App/ObjectEditorDialog.h"
#include "I18N/Text.h"
#include "Ui/MessageBox.h"
#include "Ui/FloodLayout.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.ObjectEditorDialog", ObjectEditorDialog, ui::ConfigDialog)

ObjectEditorDialog::ObjectEditorDialog(PropertyGroup* settings, const IObjectEditorFactory* objectEditorFactory)
:	m_settings(settings)
,	m_objectEditorFactory(objectEditorFactory)
,	m_objectHash(0)
,	m_modified(false)
{
}

bool ObjectEditorDialog::create(IEditor* editor, ui::Widget* parent, db::Instance* instance, ISerializable* object)
{
	// Create IEditor wrapper; need to disable dialogs when accessing editor's browsers.
	m_editor = new ObjectEditor(editor, this);
	
	// Create concrete object editor.
	m_objectEditor = m_objectEditorFactory->createObjectEditor(m_editor);
	if (!m_objectEditor)
		return false;

	ui::Size preferredSize = m_objectEditor->getPreferredSize();

	// Get instance's editor dimensions from settings.
	int32_t width = m_settings->getProperty< int32_t >(L"Editor.ObjectEditor.Dimensions/" + instance->getGuid().format() + L"/Width", preferredSize.cx);
	int32_t height = m_settings->getProperty< int32_t >(L"Editor.ObjectEditor.Dimensions/" + instance->getGuid().format() + L"/Height", preferredSize.cy);

	StringOutputStream ss;
	ss << L"Edit \"" << instance->getName() << L"\" (" << type_name(object) << L")";

	if (!ui::ConfigDialog::create(
		parent,
		ss.str(),
		width,
		height,
		ui::ConfigDialog::WsDefaultResizable | ui::ConfigDialog::WsApplyButton,
		new ui::FloodLayout()
	))
		return false;

	addEventHandler< ui::ButtonClickEvent >(this, &ObjectEditorDialog::eventClick);
	addEventHandler< ui::CloseEvent >(this, &ObjectEditorDialog::eventClose);
	addEventHandler< ui::TimerEvent >(this, &ObjectEditorDialog::eventTimer);

	m_instance = instance;
	m_objectHash = DeepHash(object).get();

	if (!m_objectEditor->create(this, instance, object))
		return false;

	startTimer(500);
	update();

	return true;
}

void ObjectEditorDialog::destroy()
{
	// Remember instance's editor dimensions in settings.
	if (m_settings && m_instance)
	{
		ui::Rect rc = getRect();
		m_settings->setProperty< PropertyInteger >(L"Editor.ObjectEditor.Dimensions/" + m_instance->getGuid().format() + L"/Width", rc.getWidth());
		m_settings->setProperty< PropertyInteger >(L"Editor.ObjectEditor.Dimensions/" + m_instance->getGuid().format() + L"/Height", rc.getHeight());
	}

	if (m_objectEditor)
	{
		m_objectEditor->destroy();
		m_objectEditor = 0;
	}

	ui::ConfigDialog::destroy();
}

bool ObjectEditorDialog::apply(bool keep)
{
	m_objectEditor->apply();
	if (m_instance->commit(keep ? db::CfKeepCheckedOut : db::CfDefault))
	{
		m_objectHash = DeepHash(m_instance->getObject()).get();
		return true;
	}
	else
	{
		ui::MessageBox::show(this, i18n::Text(L"OBJECTEDITOR_ERROR_UNABLE_TO_COMMIT_MESSAGE"), i18n::Text(L"OBJECTEDITOR_ERROR_UNABLE_TO_COMMIT_CAPTION"), ui::MbIconError | ui::MbOk);
		return false;
	}
}

void ObjectEditorDialog::cancel()
{
	destroy();
	if (m_instance)
	{
		m_instance->revert();
		m_instance = 0;
	}
}

bool ObjectEditorDialog::handleCommand(const ui::Command& command)
{
	return m_objectEditor->handleCommand(command);
}

void ObjectEditorDialog::handleDatabaseEvent(db::Database* database, const Guid& eventId)
{
	m_objectEditor->handleDatabaseEvent(database, eventId);
}

void ObjectEditorDialog::eventClick(ui::ButtonClickEvent* event)
{
	const ui::Command& command = event->getCommand();
	switch (command.getId())
	{
	case ui::DrApply:
		apply(true);
		break;

	case ui::DrOk:
		if (apply(false))
			destroy();
		break;

	case ui::DrCancel:
		cancel();
		break;
	}
}

void ObjectEditorDialog::eventClose(ui::CloseEvent* event)
{
	cancel();
	event->consume();
}

void ObjectEditorDialog::eventTimer(ui::TimerEvent* event)
{
	// Apply changes to current object.
	m_objectEditor->apply();

	// Check if object has been modified since last update.
	Ref< ISerializable > object = m_instance->getObject();
	uint32_t objectHash = DeepHash(object).get();
	bool modified = (objectHash != m_objectHash);
	if (modified != m_modified)
	{
		StringOutputStream ss;
		ss << L"Edit \"" << m_instance->getName() << L"\" (" << type_name(object) << (modified ? L")*" : L")");
		setText(ss.str());
		m_modified = modified;
	}
}

	}
}
