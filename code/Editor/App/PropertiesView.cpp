#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
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
#include "Ui/FileDialog.h"
#include "Ui/TableLayout.h"
#include "Ui/Custom/ColorPicker/ColorDialog.h"
#include "Ui/Custom/GradientStatic/GradientStatic.h"
#include "Ui/Custom/PropertyList/FilePropertyItem.h"
#include "Ui/Custom/PropertyList/BrowsePropertyItem.h"
#include "Ui/Custom/PropertyList/ObjectPropertyItem.h"
#include "Ui/Custom/PropertyList/PropertyCommandEvent.h"
#include "Ui/Custom/PropertyList/PropertyContentChangeEvent.h"
#include "Ui/Custom/PropertyList/ArrayPropertyItem.h"
#include "Ui/Custom/PropertyList/TextPropertyItem.h"
#include "Ui/Custom/PropertyList/ColorPropertyItem.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.PropertiesView", PropertiesView, ui::Container)

PropertiesView::PropertiesView(IEditor* editor)
:	m_editor(editor)
{
}

bool PropertiesView::create(ui::Widget* parent)
{
	if (!ui::Container::create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"100%,75", 0, 4)))
		return false;

	m_propertyList = new ui::custom::AutoPropertyList();
	m_propertyList->create(this, ui::WsDoubleBuffer | ui::WsTabStop, this);
	m_propertyList->addEventHandler< ui::custom::PropertyCommandEvent >(this, &PropertiesView::eventPropertyCommand);
	m_propertyList->addEventHandler< ui::custom::PropertyContentChangeEvent >(this, &PropertiesView::eventPropertyChange);
	m_propertyList->addEventHandler< ui::SelectionChangeEvent >(this, &PropertiesView::eventPropertySelect);

	m_staticHelp = new ui::custom::GradientStatic();
	m_staticHelp->create(this, Color4ub(240, 240, 240), Color4ub(255, 255, 255), Color4ub(0, 0, 0), L"", ui::WsDoubleBuffer);

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

namespace
{
	std::wstring lookupDocumentation(const std::wstring& id)
	{
		std::wstring id_;
		
		id_ = toUpper< std::wstring >(id);
		id_ = replaceAll< std::wstring >(id_, L'.', L'_');
		id_ = replaceAll< std::wstring >(id_, L' ', L'_');

		std::wstring documentation = i18n::I18N::getInstance().get(id_);
		if (documentation.empty())
			log::debug << L"No documentation \"" << id_ << L"\"" << Endl;

		return documentation;
	}
}

void PropertiesView::updateHelp()
{
	const TypeInfo* helpType = m_propertyObject ? &type_of(m_propertyObject) : 0;
	std::wstring help;

	RefArray< ui::custom::PropertyItem > selectedItems;
	if (m_propertyList->getPropertyItems(selectedItems, ui::custom::PropertyList::GfSelectedOnly | ui::custom::PropertyList::GfDescendants) == 1)
	{
		std::wstring helpPropId;

		ui::custom::PropertyItem* parent = selectedItems[0]->getParentItem();
		while (parent)
		{
			ui::custom::ObjectPropertyItem* objectItem = dynamic_type_cast< ui::custom::ObjectPropertyItem* >(parent);
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

void PropertiesView::eventPropertyCommand(ui::custom::PropertyCommandEvent* event)
{
	const ui::Command& cmd = event->getCommand();

	Ref< ui::custom::FilePropertyItem > fileItem = dynamic_type_cast< ui::custom::FilePropertyItem* >(event->getItem());
	if (fileItem)
	{
		ui::FileDialog fileDialog;
		if (!fileDialog.create(m_propertyList, i18n::Text(L"EDITOR_BROWSE_FILE"), L"All files (*.*);*.*"))
			return;

		Path path = fileItem->getPath();
		if (fileDialog.showModal(path) == ui::DrOk)
		{
			fileItem->setPath(path);
			m_propertyList->apply();
		}

		fileDialog.destroy();
	}

	Ref< ui::custom::BrowsePropertyItem > browseItem = dynamic_type_cast< ui::custom::BrowsePropertyItem* >(event->getItem());
	if (browseItem)
	{
		if (cmd == L"Property.Browse")
		{
			if (browseItem->getValue().isNull())
			{
				Ref< db::Instance > instance;
				if (browseItem->getFilterType())
				{
					const TypeInfo* filterType = browseItem->getFilterType();
					T_ASSERT (filterType);

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
		else if (cmd == L"Property.Edit")
		{
			Guid instanceGuid = browseItem->getValue();
			if (instanceGuid.isNull() || !instanceGuid.isValid())
				return;

			Ref< db::Instance > instance = m_editor->getSourceDatabase()->getInstance(instanceGuid);
			if (!instance)
				return;

			m_editor->openEditor(instance);
		}
	}

	Ref< ui::custom::ObjectPropertyItem > objectItem = dynamic_type_cast< ui::custom::ObjectPropertyItem* >(event->getItem());
	if (objectItem)
	{
		const TypeInfo* objectType = objectItem->getObjectType();
		if (!objectType)
			objectType = &type_of< ISerializable >();

		if (!objectItem->getObject())
		{
			objectType = m_editor->browseType(objectType);
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
			if (ui::custom::ArrayPropertyItem* parentArrayItem = dynamic_type_cast< ui::custom::ArrayPropertyItem* >(objectItem->getParentItem()))
				m_propertyList->removePropertyItem(parentArrayItem, objectItem);
			else
				objectItem->setObject(0);

			m_propertyList->refresh(objectItem, 0);
			m_propertyList->apply();

			m_editor->getActiveEditorPage()->handleCommand(ui::Command(L"Editor.PropertiesChanged"));
		}
	}
	
	Ref< ui::custom::ArrayPropertyItem > arrayItem = dynamic_type_cast< ui::custom::ArrayPropertyItem* >(event->getItem());
	if (arrayItem)
	{
		if (arrayItem->getElementType())
		{
			const TypeInfo* objectType = m_editor->browseType(arrayItem->getElementType());
			if (objectType)
			{
				Ref< ISerializable > object = dynamic_type_cast< ISerializable* >(objectType->createInstance());
				if (object)
				{
					m_propertyList->addObject(arrayItem, object);
					m_propertyList->apply();
					m_propertyList->refresh();

					m_editor->getActiveEditorPage()->handleCommand(ui::Command(L"Editor.PropertiesChanged"));
				}
			}
		}
		else	// Non-complex array; just apply and refresh.
		{
			m_propertyList->apply();
			m_propertyList->refresh();
			m_editor->getActiveEditorPage()->handleCommand(ui::Command(L"Editor.PropertiesChanged"));
		}
	}

	Ref< ui::custom::TextPropertyItem > textItem = dynamic_type_cast< ui::custom::TextPropertyItem* >(event->getItem());
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

	Ref< ui::custom::ColorPropertyItem > colorItem = dynamic_type_cast< ui::custom::ColorPropertyItem* >(event->getItem());
	if (colorItem)
	{
		ui::custom::ColorDialog colorDialog;
		colorDialog.create(this, i18n::Text(L"COLOR_DIALOG_TEXT"), ui::custom::ColorDialog::WsDefaultFixed | ui::custom::ColorDialog::WsAlpha, colorItem->getValue());
		if (colorDialog.showModal() == ui::DrOk)
		{
			colorItem->setValue(colorDialog.getColor());

			m_propertyList->apply();
			m_editor->getActiveEditorPage()->handleCommand(ui::Command(L"Editor.PropertiesChanged"));
		}
		colorDialog.destroy();
	}

	m_propertyList->update();
}

void PropertiesView::eventPropertyChange(ui::custom::PropertyContentChangeEvent* event)
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
