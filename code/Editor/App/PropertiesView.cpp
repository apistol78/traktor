#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyStringSet.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "Editor/IEditorPage.h"
#include "Editor/App/PropertiesView.h"
#include "Editor/App/TextEditorDialog.h"
#include "I18N/I18N.h"
#include "I18N/Text.h"
#include "Ui/Application.h"
#include "Ui/Static.h"
#include "Ui/TableLayout.h"
#include "Ui/FileDialog.h"
#include "Ui/ColorPicker/ColorDialog.h"
#include "Ui/PropertyList/FilePropertyItem.h"
#include "Ui/PropertyList/BrowsePropertyItem.h"
#include "Ui/PropertyList/ObjectPropertyItem.h"
#include "Ui/PropertyList/PropertyCommandEvent.h"
#include "Ui/PropertyList/PropertyContentChangeEvent.h"
#include "Ui/PropertyList/ArrayPropertyItem.h"
#include "Ui/PropertyList/TextPropertyItem.h"
#include "Ui/PropertyList/ColorPropertyItem.h"

namespace traktor
{
	namespace editor
	{
		namespace
		{

std::wstring lookupDocumentation(const std::wstring& id)
{
	std::wstring id_;

	id_ = toUpper(id);
	id_ = replaceAll(id_, L'.', L'_');
	id_ = replaceAll(id_, L' ', L'_');

	std::wstring documentation = i18n::I18N::getInstance().get(id_);
	if (documentation.empty())
		log::debug << L"No documentation \"" << id_ << L"\"" << Endl;

	return documentation;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.PropertiesView", PropertiesView, ui::Container)

PropertiesView::PropertiesView(IEditor* editor)
:	m_editor(editor)
{
}

bool PropertiesView::create(ui::Widget* parent)
{
	if (!ui::Container::create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"100%,75", 0, 4)))
		return false;

	m_propertyList = new ui::AutoPropertyList();
	m_propertyList->create(this, ui::WsAccelerated | ui::WsTabStop, this);
	m_propertyList->setSeparator(ui::dpi96(150));
	m_propertyList->addEventHandler< ui::PropertyCommandEvent >(this, &PropertiesView::eventPropertyCommand);
	m_propertyList->addEventHandler< ui::PropertyContentChangeEvent >(this, &PropertiesView::eventPropertyChange);
	m_propertyList->addEventHandler< ui::SelectionChangeEvent >(this, &PropertiesView::eventPropertySelect);

	m_staticHelp = new ui::Static();
	m_staticHelp->create(this, L"");

	updateHelp();
	return true;
}

void PropertiesView::destroy()
{
	safeDestroy(m_propertyList);
	ui::Container::destroy();
}

void PropertiesView::setPropertyObject(ISerializable* object)
{
	// Capture state of current property object.
	if (m_propertyObject)
		m_states[&type_of(m_propertyObject)] = m_propertyList->captureState();

	// Bind property object.
	m_propertyList->bind(object);

	// Restore state of last property object of same type.
	if (object)
	{
		std::map< const TypeInfo*, Ref< ui::HierarchicalState > >::iterator i = m_states.find(&type_of(object));
		if (i != m_states.end())
			m_propertyList->applyState(i->second);
	}

	m_propertyList->update();
	m_propertyObject = object;

	updateHelp();
}

Ref< ISerializable > PropertiesView::getPropertyObject()
{
	return m_propertyObject;
}

bool PropertiesView::handleCommand(const ui::Command& command)
{
	if (command == L"Editor.Copy")
		return m_propertyList->copy();
	else if (command == L"Editor.Paste")
	{
		if (m_propertyList->paste())
		{
			m_propertyList->apply();
			m_editor->getActiveEditorPage()->handleCommand(ui::Command(L"Editor.PropertiesChanged"));
			return true;
		}
		else
			return false;
	}
	else if (command == L"Editor.SettingsChanged")
	{
		updateHelp();
		return false;
	}
	else
		return false;
}

bool PropertiesView::resolvePropertyGuid(const Guid& guid, std::wstring& resolved) const
{
	Ref< db::Instance > instance = m_editor->getSourceDatabase()->getInstance(guid);
	if (!instance)
		return false;

	resolved = instance->getName();
	return true;
}

void PropertiesView::updateHelp()
{
	bool visible = m_editor->getSettings()->getProperty< bool >(L"Editor.PropertyHelpVisible", false);
	if (visible != m_staticHelp->isVisible(false))
	{
		m_staticHelp->setVisible(visible);
		setLayout(new ui::TableLayout(L"100%", visible ? L"100%,75" : L"100%,0", 0, 4));
		update();
	}

	if (visible)
	{
		const TypeInfo* helpType = m_propertyObject ? &type_of(m_propertyObject) : 0;
		std::wstring help;

		RefArray< ui::PropertyItem > selectedItems;
		if (m_propertyList->getPropertyItems(selectedItems, ui::PropertyList::GfSelectedOnly | ui::PropertyList::GfDescendants) == 1)
		{
			std::wstring helpPropId;

			ui::PropertyItem* parent = selectedItems[0]->getParentItem();
			while (parent)
			{
				ui::ObjectPropertyItem* objectItem = dynamic_type_cast< ui::ObjectPropertyItem* >(parent);
				if (objectItem)
				{
					helpType = objectItem->getObject() ? &type_of(objectItem->getObject()) : objectItem->getObjectType();
					break;
				}
				helpPropId = L"_" + parent->getText() + helpPropId;
				parent = parent->getParentItem();
			}

			while (helpType)
			{
				help = lookupDocumentation(L"HELP_" + std::wstring(helpType->getName()) + helpPropId + L"_" + selectedItems[0]->getText());
				if (!help.empty())
					break;

				helpType = helpType->getSuper();
			}

			if (help.empty())
				help = lookupDocumentation(L"HELP" + helpPropId + L"_" + selectedItems[0]->getText());
		}
		else if (helpType)
		{
			while (helpType)
			{
				help = lookupDocumentation(L"HELP_" + std::wstring(helpType->getName()));
				if (!help.empty())
					break;

				helpType = helpType->getSuper();
			}
		}

		m_staticHelp->setText(help);
		m_staticHelp->update();
	}
}

void PropertiesView::eventPropertyCommand(ui::PropertyCommandEvent* event)
{
	const ui::Command& cmd = event->getCommand();
	if (cmd == L"Property.Add")
	{
		ui::ArrayPropertyItem* arrayItem = dynamic_type_cast< ui::ArrayPropertyItem* >(event->getItem());
		if (arrayItem)
		{
			if (arrayItem->getElementType())
			{
				Ref< ITypedObject > object = cmd.getData();
				if (!object)
				{
					const TypeInfo* objectType = m_editor->browseType(makeTypeInfoSet(*arrayItem->getElementType()), false, true);
					if (objectType)
						object = objectType->createInstance();
				}
				if (object)
				{
					m_propertyList->addObject(arrayItem, mandatory_non_null_type_cast< ISerializable* >(object));
					m_propertyList->apply();
					m_propertyList->refresh();
					m_editor->getActiveEditorPage()->handleCommand(ui::Command(L"Editor.PropertiesChanged"));
				}
			}
			else	// Non-complex array; just apply and refresh.
			{
				m_propertyList->apply();
				m_propertyList->refresh();
				m_editor->getActiveEditorPage()->handleCommand(ui::Command(L"Editor.PropertiesChanged"));
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
			m_editor->getActiveEditorPage()->handleCommand(ui::Command(L"Editor.PropertiesChanged"));
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
					m_editor->getActiveEditorPage()->handleCommand(ui::Command(L"Editor.PropertiesChanged"));
				}
			}
			else
			{
				browseItem->setValue(Guid());
				m_propertyList->apply();
				m_editor->getActiveEditorPage()->handleCommand(ui::Command(L"Editor.PropertiesChanged"));
			}
		}

		ui::FilePropertyItem* fileItem = dynamic_type_cast< ui::FilePropertyItem* >(event->getItem());
		if (fileItem)
		{
			ui::FileDialog fileDialog;
			if (!fileDialog.create(m_propertyList, type_name(this), i18n::Text(L"EDITOR_BROWSE_FILE"), L"All files (*.*);*.*"))
				return;

			Path path = fileItem->getPath();
			if (fileDialog.showModal(path) == ui::DrOk)
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

						m_editor->getActiveEditorPage()->handleCommand(ui::Command(L"Editor.PropertiesChanged"));
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

				m_editor->getActiveEditorPage()->handleCommand(ui::Command(L"Editor.PropertiesChanged"));
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
			textEditorDialog.create(this, textItem->getValue());
			if (textEditorDialog.showModal() == ui::DrOk)
			{
				textItem->setValue(textEditorDialog.getText());

				m_propertyList->apply();
				m_editor->getActiveEditorPage()->handleCommand(ui::Command(L"Editor.PropertiesChanged"));
			}
			textEditorDialog.destroy();
		}

		ui::ColorPropertyItem* colorItem = dynamic_type_cast< ui::ColorPropertyItem* >(event->getItem());
		if (colorItem)
		{
			ui::ColorDialog colorDialog;
			colorDialog.create(
				this,
				i18n::Text(L"COLOR_DIALOG_TEXT"),
				ui::ColorDialog::WsDefaultFixed | ui::ColorDialog::WsAlpha | (colorItem->getHighDynamicRange() ? ui::ColorDialog::WsHDR : 0),
				colorItem->getValue()
			);
			if (colorDialog.showModal() == ui::DrOk)
			{
				colorItem->setValue(colorDialog.getColor());

				m_propertyList->apply();
				m_editor->getActiveEditorPage()->handleCommand(ui::Command(L"Editor.PropertiesChanged"));
			}
			colorDialog.destroy();
		}
	}
	m_propertyList->update();
}

void PropertiesView::eventPropertyChange(ui::PropertyContentChangeEvent* event)
{
	IEditorPage* activeEditorPage = m_editor->getActiveEditorPage();
	if (activeEditorPage)
		activeEditorPage->handleCommand(ui::Command(L"Editor.PropertiesChanging"));

	m_propertyList->apply();

	if (activeEditorPage)
		activeEditorPage->handleCommand(ui::Command(L"Editor.PropertiesChanged"));
}

void PropertiesView::eventPropertySelect(ui::SelectionChangeEvent* event)
{
	updateHelp();
}

	}
}
