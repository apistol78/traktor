#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyStringSet.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "Editor/App/DefaultObjectEditor.h"
#include "Editor/App/TextEditorDialog.h"
#include "I18N/Text.h"
#include "Ui/Application.h"
#include "Ui/Event.h"
#include "Ui/FileDialog.h"
#include "Ui/ColorPicker/ColorDialog.h"
#include "Ui/PropertyList/ArrayPropertyItem.h"
#include "Ui/PropertyList/BrowsePropertyItem.h"
#include "Ui/PropertyList/ColorPropertyItem.h"
#include "Ui/PropertyList/FilePropertyItem.h"
#include "Ui/PropertyList/ObjectPropertyItem.h"
#include "Ui/PropertyList/PropertyCommandEvent.h"
#include "Ui/PropertyList/TextPropertyItem.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.DefaultObjectEditor", DefaultObjectEditor, IObjectEditor)

DefaultObjectEditor::DefaultObjectEditor(IEditor* editor)
:	m_editor(editor)
{
}

bool DefaultObjectEditor::create(ui::Widget* parent, db::Instance* instance, ISerializable* object)
{
	m_instance = instance;
	m_object = object;

	m_propertyList = new ui::AutoPropertyList();
	m_propertyList->create(parent, ui::WsAccelerated | ui::WsTabStop | ui::AutoPropertyList::WsColumnHeader, this);
	m_propertyList->addEventHandler< ui::PropertyCommandEvent >(this, &DefaultObjectEditor::eventPropertyCommand);
	m_propertyList->setSeparator(ui::dpi96(200));
	m_propertyList->setColumnName(0, i18n::Text(L"PROPERTY_COLUMN_NAME"));
	m_propertyList->setColumnName(1, i18n::Text(L"PROPERTY_COLUMN_VALUE"));
	m_propertyList->bind(m_object);

	return true;
}

void DefaultObjectEditor::destroy()
{
	safeDestroy(m_propertyList);
	m_instance = 0;
	m_object = 0;
}

void DefaultObjectEditor::apply()
{
	m_propertyList->apply();
	m_instance->setObject(m_object);
}

bool DefaultObjectEditor::handleCommand(const ui::Command& command)
{
	if (command == L"Editor.Copy")
		return m_propertyList->copy();
	else if (command == L"Editor.Paste")
	{
		if (m_propertyList->paste())
		{
			m_propertyList->apply();
			return true;
		}
		else
			return false;
	}
	else
		return false;
}

void DefaultObjectEditor::handleDatabaseEvent(db::Database* database, const Guid& eventId)
{
}

ui::Size DefaultObjectEditor::getPreferredSize() const
{
	return ui::Size(ui::dpi96(600), ui::dpi96(500));
}

bool DefaultObjectEditor::resolvePropertyGuid(const Guid& guid, std::wstring& resolved) const
{
	Ref< db::Instance > instance = m_editor->getSourceDatabase()->getInstance(guid);
	if (!instance)
		return false;

	resolved = instance->getPath();
	return true;
}

void DefaultObjectEditor::eventPropertyCommand(ui::PropertyCommandEvent* event)
{
	const ui::Command& cmd = event->getCommand();
	if (cmd == L"Property.Add")
	{
		ui::ArrayPropertyItem* arrayItem = dynamic_type_cast< ui::ArrayPropertyItem* >(event->getItem());
		if (arrayItem)
		{
			if (arrayItem->getElementType())
			{
				const TypeInfo* objectType = m_editor->browseType(makeTypeInfoSet(*arrayItem->getElementType()), false, true);
				if (objectType)
				{
					Ref< ISerializable > object = dynamic_type_cast< ISerializable* >(objectType->createInstance());
					if (object)
					{
						m_propertyList->addObject(arrayItem, object);
						m_propertyList->apply();
						m_propertyList->refresh();
					}
				}
			}
			else	// Non-complex array; just apply and refresh.
			{
				m_propertyList->apply();
				m_propertyList->refresh();
			}
		}
	}
	else if (cmd == L"Property.Remove")
	{
		ui::PropertyItem* removeItem = event->getItem();
		ui::PropertyItem* parentItem = removeItem->getParentItem();
		if (parentItem)
		{
			m_propertyList->removePropertyItem(parentItem, removeItem);
			m_propertyList->apply();
		}
	}
	else if (cmd == L"Property.Browse")
	{
		ui::BrowsePropertyItem* browseItem = dynamic_type_cast< ui::BrowsePropertyItem* >(event->getItem());
		if (browseItem)
		{
			if (browseItem->getValue().isNull())
			{
				Ref< db::Instance > instance;
				if (browseItem->getFilterType())
				{
					const TypeInfo* filterType = browseItem->getFilterType();
					T_ASSERT(filterType);

					instance = m_editor->browseInstance(*filterType);
				}
				else
					instance = m_editor->browseInstance();

				if (instance)
				{
					browseItem->setValue(instance->getGuid());
					m_propertyList->apply();
				}
			}
			else
			{
				browseItem->setValue(Guid());
				m_propertyList->apply();
			}
		}

		ui::FilePropertyItem* fileItem = dynamic_type_cast< ui::FilePropertyItem* >(event->getItem());
		if (fileItem)
		{
			ui::FileDialog fileDialog;
			if (!fileDialog.create(m_propertyList, type_name(this), i18n::Text(L"EDITOR_BROWSE_FILE"), L"All files (*.*);*.*"))
				return;

			Path path = fileItem->getPath();
			if (fileDialog.showModal(path) == ui::DialogResult::Ok)
			{
				fileItem->setPath(path);
				m_propertyList->apply();
			}

			fileDialog.destroy();
		}

		ui::ObjectPropertyItem* objectItem = dynamic_type_cast< ui::ObjectPropertyItem* >(event->getItem());
		if (objectItem)
		{
			const TypeInfo* objectType = objectItem->getObjectType();
			if (!objectType)
				objectType = &type_of< ISerializable >();

			if (!objectItem->getObject())
			{
				objectType = m_editor->browseType(makeTypeInfoSet(*objectType), false, true);
				if (objectType)
				{
					Ref< ISerializable > object = dynamic_type_cast< ISerializable* >(objectType->createInstance());
					if (object)
					{
						objectItem->setObject(object);

						m_propertyList->refresh(objectItem, object);
						m_propertyList->apply();
					}
				}
			}
			else
			{
				if (ui::ArrayPropertyItem* parentArrayItem = dynamic_type_cast< ui::ArrayPropertyItem* >(objectItem->getParentItem()))
					m_propertyList->removePropertyItem(parentArrayItem, objectItem);
				else
					objectItem->setObject(0);

				m_propertyList->refresh(objectItem, 0);
				m_propertyList->apply();
			}
		}
	}
	else if (cmd == L"Property.Edit")
	{
		ui::BrowsePropertyItem* browseItem = dynamic_type_cast< ui::BrowsePropertyItem* >(event->getItem());
		if (browseItem)
		{
			Guid instanceGuid = browseItem->getValue();
			if (instanceGuid.isNull() || !instanceGuid.isValid())
				return;

			Ref< db::Instance > instance = m_editor->getSourceDatabase()->getInstance(instanceGuid);
			if (!instance)
				return;

			m_editor->openEditor(instance);
		}

		ui::TextPropertyItem* textItem = dynamic_type_cast< ui::TextPropertyItem* >(event->getItem());
		if (textItem)
		{
			TextEditorDialog textEditorDialog;
			textEditorDialog.create(m_propertyList, textItem->getValue());
			if (textEditorDialog.showModal() == ui::DialogResult::Ok)
			{
				textItem->setValue(textEditorDialog.getText());
				m_propertyList->apply();
			}
			textEditorDialog.destroy();
		}

		ui::ColorPropertyItem* colorItem = dynamic_type_cast< ui::ColorPropertyItem* >(event->getItem());
		if (colorItem)
		{
			ui::ColorDialog colorDialog;
			colorDialog.create(m_propertyList, i18n::Text(L"COLOR_DIALOG_TEXT"), ui::ColorDialog::WsDefaultFixed | ui::ColorDialog::WsAlpha, colorItem->getValue());
			if (colorDialog.showModal() == ui::DialogResult::Ok)
			{
				colorItem->setValue(colorDialog.getColor());
				m_propertyList->apply();
			}
			colorDialog.destroy();
		}
	}
	m_propertyList->update();
}

	}
}
