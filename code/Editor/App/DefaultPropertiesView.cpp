/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyStringSet.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "Editor/IEditorPage.h"
#include "Editor/App/DefaultPropertiesView.h"
#include "Editor/App/TextEditorDialog.h"
#include "I18N/I18N.h"
#include "I18N/Text.h"
#include "Ui/Application.h"
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

namespace traktor::editor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.DefaultPropertiesView", DefaultPropertiesView, PropertiesView)

DefaultPropertiesView::DefaultPropertiesView(IEditor* editor)
:	m_editor(editor)
{
}

bool DefaultPropertiesView::create(ui::Widget* parent)
{
	if (!ui::Widget::create(parent, ui::WsNone))
		return false;
	addEventHandler< ui::SizeEvent >(this, &DefaultPropertiesView::eventSize);

	setText(i18n::Text(L"TITLE_PROPERTIES"));

	const int32_t separator = m_editor->getSettings()->getProperty< int32_t >(L"Editor.DefaultPropertiesView.Separator", 180);

	m_propertyList = new ui::AutoPropertyList();
	m_propertyList->create(this, ui::WsAccelerated | ui::WsTabStop, this);
	m_propertyList->setSeparator(ui::Unit(separator));
	m_propertyList->addEventHandler< ui::PropertyCommandEvent >(this, &DefaultPropertiesView::eventPropertyCommand);
	m_propertyList->addEventHandler< ui::PropertyContentChangeEvent >(this, &DefaultPropertiesView::eventPropertyChange);
	return true;
}

void DefaultPropertiesView::destroy()
{
	if (m_editor)
	{
		Ref< PropertyGroup > settings = m_editor->checkoutGlobalSettings();
		if (settings)
		{
			const int32_t separator = m_propertyList->getSeparator().get();
			settings->setProperty< PropertyInteger >(L"Editor.DefaultPropertiesView.Separator", separator);
			m_editor->commitGlobalSettings();
		}
	}
	PropertiesView::destroy();
}

void DefaultPropertiesView::update(const ui::Rect* rc, bool immediate)
{
	if (m_propertyList)
	{
		const ui::Rect rcInner = getInnerRect();
		m_propertyList->setRect(rcInner);
	}
	Widget::update(rc, immediate);
}

void DefaultPropertiesView::setPropertyObject(ISerializable* object)
{
	// Capture state of current property object.
	if (m_propertyObject)
		m_states[&type_of(m_propertyObject)] = m_propertyList->captureState();

	// Bind property object.
	m_propertyList->bind(object);

	// Restore state of last property object of same type.
	if (object)
	{
		auto it = m_states.find(&type_of(object));
		if (it != m_states.end())
			m_propertyList->applyState(it->second);
	}

	m_propertyList->update();
	m_propertyObject = object;
}

bool DefaultPropertiesView::handleCommand(const ui::Command& command)
{
	if (!m_propertyList->containFocus())
		return false;

	if (command == L"Editor.Copy")
		return m_propertyList->copy();
	else if (command == L"Editor.Paste")
	{
		if (m_propertyList->paste())
		{
			m_propertyList->apply();

			ui::ContentChangeEvent event(this);
			raiseEvent(&event);
			return true;
		}
	}

	return false;
}

bool DefaultPropertiesView::resolvePropertyGuid(const Guid& guid, std::wstring& resolved) const
{
	Ref< db::Instance > instance = m_editor->getSourceDatabase()->getInstance(guid);
	if (!instance)
		return false;

	resolved = instance->getName();
	return true;
}

void DefaultPropertiesView::eventSize(ui::SizeEvent* event)
{
	update(nullptr, false);
}

void DefaultPropertiesView::eventPropertyCommand(ui::PropertyCommandEvent* event)
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

					ui::ContentChangeEvent event(this);
					raiseEvent(&event);
				}
			}
			else	// Non-complex array; just apply and refresh.
			{
				m_propertyList->apply();
				m_propertyList->refresh();

				ui::ContentChangeEvent event(this);
				raiseEvent(&event);
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

			ui::ContentChangeEvent event(this);
			raiseEvent(&event);
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

					ui::ContentChangeEvent event(this);
					raiseEvent(&event);
				}
			}
			else
			{
				browseItem->setValue(Guid());
				m_propertyList->apply();

				ui::ContentChangeEvent event(this);
				raiseEvent(&event);
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

						ui::ContentChangeEvent event(this);
						raiseEvent(&event);
					}
				}
			}
			else
			{
				if (ui::ArrayPropertyItem* parentArrayItem = dynamic_type_cast< ui::ArrayPropertyItem* >(objectItem->getParentItem()))
					m_propertyList->removePropertyItem(parentArrayItem, objectItem);
				else
					objectItem->setObject(nullptr);

				m_propertyList->refresh(objectItem, nullptr);
				m_propertyList->apply();

				ui::ContentChangeEvent event(this);
				raiseEvent(&event);
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
			if (textEditorDialog.showModal() == ui::DialogResult::Ok)
			{
				textItem->setValue(textEditorDialog.getText());
				m_propertyList->apply();

				ui::PropertyContentChangeEvent event(this, textItem);
				raiseEvent(&event);
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
			if (colorDialog.showModal() == ui::DialogResult::Ok)
			{
				colorItem->setValue(colorDialog.getColor());
				m_propertyList->apply();

				ui::PropertyContentChangeEvent event(this, colorItem);
				raiseEvent(&event);
			}
			colorDialog.destroy();
		}
	}
	m_propertyList->update();
}

void DefaultPropertiesView::eventPropertyChange(ui::PropertyContentChangeEvent* event)
{
	ui::ContentChangingEvent changingEvent(this);
	raiseEvent(&changingEvent);

	m_propertyList->apply();

	ui::PropertyContentChangeEvent changeEvent(this, event->getItem());
	raiseEvent(&changeEvent);	
}

}
