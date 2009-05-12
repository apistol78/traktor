#include "Editor/App/ObjectEditorDialog.h"
#include "Editor/ObjectEditor.h"
#include "Ui/MessageBox.h"
#include "Ui/FloodLayout.h"
#include "Ui/MethodHandler.h"
#include "Ui/Events/CommandEvent.h"
#include "I18N/Text.h"
#include "Database/Instance.h"
#include "Core/Io/StringOutputStream.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.ObjectEditorDialog", ObjectEditorDialog, ui::ConfigDialog)

ObjectEditorDialog::ObjectEditorDialog(ObjectEditor* objectEditor)
:	m_objectEditor(objectEditor)
{
}

bool ObjectEditorDialog::create(ui::Widget* parent, db::Instance* instance, Object* object)
{
	StringOutputStream ss;
	ss << L"Edit \"" << instance->getName() << L"\"";

	if (!ui::ConfigDialog::create(parent, ss.str(), 500, 400, ui::ConfigDialog::WsDefaultResizable | ui::ConfigDialog::WsApplyButton, gc_new< ui::FloodLayout >()))
		return false;

	addClickEventHandler(ui::createMethodHandler(this, &ObjectEditorDialog::eventClick));
	addCloseEventHandler(ui::createMethodHandler(this, &ObjectEditorDialog::eventClose));

	m_instance = instance;

	if (!m_objectEditor->create(this, instance, object))
		return false;

	update();

	return true;
}

void ObjectEditorDialog::destroy()
{
	if (m_objectEditor)
	{
		m_objectEditor->destroy();
		m_objectEditor = 0;
	}

	ui::ConfigDialog::destroy();
}

void ObjectEditorDialog::eventClick(ui::Event* event)
{
	const ui::Command& command = checked_type_cast< ui::CommandEvent* >(event)->getCommand();
	switch (command.getId())
	{
	case ui::DrApply:
		m_objectEditor->apply();
		if (!m_instance->commit(db::CfKeepCheckedOut))
			ui::MessageBox::show(this, i18n::Text(L"OBJECTEDITOR_ERROR_UNABLE_TO_COMMIT_MESSAGE"), i18n::Text(L"OBJECTEDITOR_ERROR_UNABLE_TO_COMMIT_CAPTION"), ui::MbIconError | ui::MbOk);
		break;

	case ui::DrOk:
		m_objectEditor->apply();
		if (m_instance->commit())
			destroy();
		else
			ui::MessageBox::show(this, i18n::Text(L"OBJECTEDITOR_ERROR_UNABLE_TO_COMMIT_MESSAGE"), i18n::Text(L"OBJECTEDITOR_ERROR_UNABLE_TO_COMMIT_CAPTION"), ui::MbIconError | ui::MbOk);
		break;

	case ui::DrCancel:
		m_instance->revert();
		destroy();
		break;
	}
}

void ObjectEditorDialog::eventClose(ui::Event* event)
{
	m_instance->revert();
	destroy();
}

	}
}
