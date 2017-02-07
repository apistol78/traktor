#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyStringSet.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "I18N/Text.h"
#include "Ui/Custom/PropertyList/ArrayPropertyItem.h"
#include "Ui/Custom/PropertyList/BrowsePropertyItem.h"
#include "Ui/Custom/PropertyList/ObjectPropertyItem.h"
#include "Ui/Custom/PropertyList/PropertyCommandEvent.h"
#include "Ui/Custom/PropertyList/PropertyContentChangeEvent.h"
#include "Render/ImageProcess/ImageProcessStep.h"
#include "Render/Editor/ImageProcess/ImageProcessProperties.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ImageProcessProperties", ImageProcessProperties, ui::EventSubject)

ImageProcessProperties::ImageProcessProperties(editor::IEditor* editor)
:	m_editor(editor)
{
}

bool ImageProcessProperties::create(ui::Widget* parent)
{
	m_propertyList = new ui::custom::AutoPropertyList();
	m_propertyList->create(parent, ui::WsDoubleBuffer | ui::custom::AutoPropertyList::WsColumnHeader, this);
	m_propertyList->addEventHandler< ui::custom::PropertyCommandEvent >(this, &ImageProcessProperties::eventPropertyCommand);
	m_propertyList->addEventHandler< ui::custom::PropertyContentChangeEvent >(this, &ImageProcessProperties::eventPropertyChange);
	m_propertyList->setSeparator(200);
	m_propertyList->setColumnName(0, i18n::Text(L"PROPERTY_COLUMN_NAME"));
	m_propertyList->setColumnName(1, i18n::Text(L"PROPERTY_COLUMN_VALUE"));
	return true;
}

void ImageProcessProperties::destroy()
{
	safeDestroy(m_propertyList);
}

void ImageProcessProperties::set(ISerializable* object)
{
	m_propertyList->bind(object);
}

bool ImageProcessProperties::resolvePropertyGuid(const Guid& guid, std::wstring& resolved) const
{
	Ref< db::Instance > instance = m_editor->getSourceDatabase()->getInstance(guid);
	if (!instance)
		return false;

	resolved = instance->getName();
	return true;
}

void ImageProcessProperties::eventPropertyCommand(ui::custom::PropertyCommandEvent* event)
{
	const ui::Command& cmd = event->getCommand();
	if (cmd == L"Property.Add")
	{
		ui::custom::ArrayPropertyItem* arrayItem = dynamic_type_cast< ui::custom::ArrayPropertyItem* >(event->getItem());
		if (arrayItem)
		{
			if (arrayItem->getElementType())
			{
				const TypeInfo* objectType = m_editor->browseType(makeTypeInfoSet(*arrayItem->getElementType()));
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
		ui::custom::PropertyItem* removeItem = event->getItem();
		ui::custom::PropertyItem* parentItem = removeItem->getParentItem();
		if (parentItem)
		{
			m_propertyList->removePropertyItem(parentItem, removeItem);
			m_propertyList->apply();
		}
	}
	else if (cmd == L"Property.Browse")
	{
		ui::custom::BrowsePropertyItem* browseItem = dynamic_type_cast< ui::custom::BrowsePropertyItem* >(event->getItem());
		if (browseItem)
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
				}
			}
			else
			{
				browseItem->setValue(Guid());
				m_propertyList->apply();
			}
		}

		/*
		ui::custom::FilePropertyItem* fileItem = dynamic_type_cast< ui::custom::FilePropertyItem* >(event->getItem());
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
		*/

		ui::custom::ObjectPropertyItem* objectItem = dynamic_type_cast< ui::custom::ObjectPropertyItem* >(event->getItem());
		if (objectItem)
		{
			const TypeInfo* objectType = objectItem->getObjectType();
			if (!objectType)
				objectType = &type_of< ISerializable >();

			if (!objectItem->getObject())
			{
				objectType = m_editor->browseType(makeTypeInfoSet(*objectType));
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
				if (ui::custom::ArrayPropertyItem* parentArrayItem = dynamic_type_cast< ui::custom::ArrayPropertyItem* >(objectItem->getParentItem()))
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
		ui::custom::BrowsePropertyItem* browseItem = dynamic_type_cast< ui::custom::BrowsePropertyItem* >(event->getItem());
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

/*
		ui::custom::TextPropertyItem* textItem = dynamic_type_cast< ui::custom::TextPropertyItem* >(event->getItem());
		if (textItem)
		{
			TextEditorDialog textEditorDialog;
			textEditorDialog.create(m_propertyList, textItem->getValue());
			if (textEditorDialog.showModal() == ui::DrOk)
			{
				textItem->setValue(textEditorDialog.getText());
				m_propertyList->apply();
			}
			textEditorDialog.destroy();
		}

		ui::custom::ColorPropertyItem* colorItem = dynamic_type_cast< ui::custom::ColorPropertyItem* >(event->getItem());
		if (colorItem)
		{
			ui::custom::ColorDialog colorDialog;
			colorDialog.create(m_propertyList, i18n::Text(L"COLOR_DIALOG_TEXT"), ui::custom::ColorDialog::WsDefaultFixed | ui::custom::ColorDialog::WsAlpha, colorItem->getValue());
			if (colorDialog.showModal() == ui::DrOk)
			{
				colorItem->setValue(colorDialog.getColor());
				m_propertyList->apply();
			}
			colorDialog.destroy();
		}
*/
	}
	m_propertyList->update();
}

void ImageProcessProperties::eventPropertyChange(ui::custom::PropertyContentChangeEvent* event)
{
	m_propertyList->apply();

	ui::ContentChangeEvent contentChangeEvent(this);
	raiseEvent(&contentChangeEvent);
}

	}
}
