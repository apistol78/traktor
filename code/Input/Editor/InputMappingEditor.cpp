#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyObject.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "Input/Binding/InputMappingSourceData.h"
#include "Input/Editor/InputMappingEditor.h"
#include "Ui/Button.h"
#include "Ui/Container.h"
#include "Ui/FileDialog.h"
#include "Ui/MethodHandler.h"
#include "Ui/TableLayout.h"
#include "Ui/Custom/PropertyList/AutoPropertyList.h"
#include "Xml/XmlDeserializer.h"
#include "Xml/XmlSerializer.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.InputMappingEditor", InputMappingEditor, editor::IObjectEditor)

InputMappingEditor::InputMappingEditor(editor::IEditor* editor)
:	m_editor(editor)
{
}

bool InputMappingEditor::create(ui::Widget* parent, db::Instance* instance, ISerializable* object)
{
	m_instance = instance;
	if (!m_instance)
		return false;

	m_mapping = dynamic_type_cast< InputMappingSourceData* >(object);
	if (!m_mapping)
		return false;

	m_containerOuter = new ui::Container();
	if (!m_containerOuter->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"100%,*", 4, 4)))
		return false;

	m_propertyListInput = new ui::custom::AutoPropertyList();
	m_propertyListInput->create(m_containerOuter, ui::WsDoubleBuffer | ui::WsBorder | ui::custom::AutoPropertyList::WsColumnHeader);
	m_propertyListInput->setSeparator(200);
	m_propertyListInput->setColumnName(0, L"Name"); //i18n::Text(L"PROPERTY_COLUMN_NAME"));
	m_propertyListInput->setColumnName(1, L"Value"); //i18n::Text(L"PROPERTY_COLUMN_VALUE"));

	Ref< ui::Container > containerImportExportInput = new ui::Container();
	containerImportExportInput->create(m_containerOuter, ui::WsNone, new ui::TableLayout(L"*,*", L"*", 0, 4));

	Ref< ui::Button > buttonImportInput = new ui::Button();
	buttonImportInput->create(containerImportExportInput, L"Import...");
	buttonImportInput->addClickEventHandler(ui::createMethodHandler(this, &InputMappingEditor::eventButtonImportInput));

	Ref< ui::Button > buttonExportInput = new ui::Button();
	buttonExportInput->create(containerImportExportInput, L"Export...");
	buttonExportInput->addClickEventHandler(ui::createMethodHandler(this, &InputMappingEditor::eventButtonExportInput));

	m_propertyListInput->bind(m_mapping);

	return true;
}

void InputMappingEditor::destroy()
{
}

void InputMappingEditor::apply()
{
	m_instance->setObject(m_mapping);
}

bool InputMappingEditor::handleCommand(const ui::Command& command)
{
	return false;
}

void InputMappingEditor::handleDatabaseEvent(db::Database* database, const Guid& eventId)
{
}

ui::Size InputMappingEditor::getPreferredSize() const
{
	return ui::Size(500, 400);
}

void InputMappingEditor::eventButtonImportInput(ui::Event* event)
{
	ui::FileDialog fileDialog;
	if (!fileDialog.create(m_containerOuter, L"Import input", L"All files (*.*);*.*"))
		return;

	Path filePath;
	if (fileDialog.showModal(filePath) == ui::DrOk)
	{
		Ref< IStream > file = FileSystem::getInstance().open(filePath, File::FmRead);
		if (file)
		{
			Ref< PropertyGroup > inputConfig = xml::XmlDeserializer(file).readObject< PropertyGroup >();
			if (inputConfig)
			{
				Ref< input::InputMappingSourceData > importedInput = dynamic_type_cast< input::InputMappingSourceData* >(inputConfig->getProperty< PropertyObject >(L"Input.Sources"));
				if (importedInput)
				{
					m_mapping = importedInput;
					m_instance->setObject(m_mapping);
					m_propertyListInput->bind(m_mapping);
				}
				else
					log::error << L"Unable to import input mapping; no \"Input.Sources\" entry found in imported settings" << Endl;
			}
			else
				log::error << L"Unable to import input mapping; failed to read file" << Endl;

			file->close();
		}
		else
			log::error << L"Unable to import input mapping; failed to open file" << Endl;
	}

	fileDialog.destroy();
}

void InputMappingEditor::eventButtonExportInput(ui::Event* event)
{
	ui::FileDialog fileDialog;
	if (!fileDialog.create(m_containerOuter, L"Export input", L"All files (*.*);*.*", true))
		return;

	Path filePath;
	if (fileDialog.showModal(filePath) == ui::DrOk)
	{
		Ref< IStream > file = FileSystem::getInstance().open(filePath, File::FmWrite);
		if (file)
		{
			Ref< PropertyGroup > inputConfig = new PropertyGroup();
			inputConfig->setProperty< PropertyObject >(L"Input.Sources", m_mapping);

			if (!xml::XmlSerializer(file).writeObject(inputConfig))
				log::error << L"Unable to export input mapping; failed to write file" << Endl;

			file->close();
		}
		else
			log::error << L"Unable to export input mapping; failed to create file" << Endl;
	}

	fileDialog.destroy();
}

	}
}
