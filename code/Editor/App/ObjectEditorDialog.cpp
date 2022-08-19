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
#include "Ui/FloodLayout.h"
#include "Ui/MessageBox.h"
#include "Ui/StyleBitmap.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.ObjectEditorDialog", ObjectEditorDialog, ui::ConfigDialog)

ObjectEditorDialog::ObjectEditorDialog(const IObjectEditorFactory* objectEditorFactory)
:	m_objectEditorFactory(objectEditorFactory)
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
	auto settings = editor->getSettings();
	int32_t x = settings->getProperty< int32_t >(L"Editor.ObjectEditor.Dimensions/" + instance->getGuid().format() + L"/X", -1);
	int32_t y = settings->getProperty< int32_t >(L"Editor.ObjectEditor.Dimensions/" + instance->getGuid().format() + L"/Y", -1);
	int32_t width = settings->getProperty< int32_t >(L"Editor.ObjectEditor.Dimensions/" + instance->getGuid().format() + L"/Width", preferredSize.cx);
	int32_t height = settings->getProperty< int32_t >(L"Editor.ObjectEditor.Dimensions/" + instance->getGuid().format() + L"/Height", preferredSize.cy);

	// In case we don't have an explicit position then center on top of parent.
	int32_t style = ui::ConfigDialog::WsDefaultResizable | ui::ConfigDialog::WsApplyButton;
	if (x < 0 || y < 0)
		style |= ui::ConfigDialog::WsCenterParent;

	StringOutputStream ss;
	ss << L"Edit \"" << instance->getName() << L"\" (" << type_name(object) << L")";

	if (!ui::ConfigDialog::create(
		parent,
		ss.str(),
		width,
		height,
		style,
		new ui::FloodLayout()
	))
		return false;

	setIcon(new ui::StyleBitmap(L"Editor.Icon"));

	addEventHandler< ui::ButtonClickEvent >(this, &ObjectEditorDialog::eventClick);
	addEventHandler< ui::CloseEvent >(this, &ObjectEditorDialog::eventClose);
	addEventHandler< ui::TimerEvent >(this, &ObjectEditorDialog::eventTimer);

	// Move dialog to same location as last time if position is given.
	if (x >= 0 && y >= 0)
	{
		setRect(ui::Rect(
			ui::Point(x, y),
			ui::Size(width, height)
		));
	}

	m_instance = instance;
	m_instanceGuid = instance->getGuid();
	m_objectHash = DeepHash(object).get();

	if (!m_objectEditor->create(this, instance, object))
		return false;

	startTimer(1000);
	update();

	return true;
}

void ObjectEditorDialog::destroy()
{
	// Remember instance's editor dimensions in settings.
	auto settings = m_editor->checkoutGlobalSettings();
	if (settings)
	{
		ui::Rect rc = getRect();
		settings->setProperty< PropertyInteger >(L"Editor.ObjectEditor.Dimensions/" + m_instanceGuid.format() + L"/X", rc.left);
		settings->setProperty< PropertyInteger >(L"Editor.ObjectEditor.Dimensions/" + m_instanceGuid.format() + L"/Y", rc.top);
		settings->setProperty< PropertyInteger >(L"Editor.ObjectEditor.Dimensions/" + m_instanceGuid.format() + L"/Width", rc.getWidth());
		settings->setProperty< PropertyInteger >(L"Editor.ObjectEditor.Dimensions/" + m_instanceGuid.format() + L"/Height", rc.getHeight());
		m_editor->commitGlobalSettings();
	}

	if (m_objectEditor)
	{
		m_objectEditor->destroy();
		m_objectEditor = nullptr;
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

	// Revert changes from instance when cancelling dialog.
	if (m_instance)
	{
		m_instance->revert();
		m_instance = nullptr;
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
	switch ((ui::DialogResult)command.getId())
	{
	case ui::DialogResult::Apply:
		apply(true);
		break;

	case ui::DialogResult::Ok:
		if (apply(false))
			destroy();
		break;

	case ui::DialogResult::Cancel:
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
