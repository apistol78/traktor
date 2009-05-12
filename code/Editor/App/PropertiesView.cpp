#include "Editor/App/PropertiesView.h"
#include "Editor/App/TextEditorDialog.h"
#include "Editor/Editor.h"
#include "Editor/EditorPage.h"
#include "Editor/Asset.h"
#include "Editor/TypeBrowseFilter.h"
#include "Ui/FloodLayout.h"
#include "Ui/MethodHandler.h"
#include "Ui/Event.h"
#include "Ui/Custom/PropertyList/BrowsePropertyItem.h"
#include "Ui/Custom/PropertyList/ObjectPropertyItem.h"
#include "Ui/Custom/PropertyList/ArrayPropertyItem.h"
#include "Ui/Custom/PropertyList/TextPropertyItem.h"
#include "Ui/Custom/PropertyList/ColorPropertyItem.h"
#include "Ui/Custom/ColorPicker/ColorDialog.h"
#include "I18N/Text.h"
#include "Database/Database.h"
#include "Database/Instance.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.PropertiesView", PropertiesView, ui::Container)

PropertiesView::PropertiesView(Editor* editor)
:	m_editor(editor)
{
}

bool PropertiesView::create(ui::Widget* parent)
{
	if (!ui::Container::create(parent, ui::WsNone, gc_new< ui::FloodLayout >()))
		return false;

	m_propertyList = gc_new< ui::custom::AutoPropertyList >();
	m_propertyList->create(this, ui::WsDoubleBuffer, this);
	m_propertyList->addCommandEventHandler(ui::createMethodHandler(this, &PropertiesView::eventPropertyCommand));
	m_propertyList->addChangeEventHandler(ui::createMethodHandler(this, &PropertiesView::eventPropertyChange));

	return true;
}

void PropertiesView::destroy()
{
	m_propertyList->destroy();
	m_propertyObject = 0;

	ui::Container::destroy();
}

void PropertiesView::setPropertyObject(Object* propertyObject)
{
	m_propertyList->bind(dynamic_type_cast< Serializable* >(propertyObject));
	m_propertyList->update();

	m_propertyObject = propertyObject;
}

Object* PropertiesView::getPropertyObject()
{
	return m_propertyObject;
}

bool PropertiesView::resolvePropertyGuid(const Guid& guid, std::wstring& resolved) const
{
	Ref< db::Instance > instance = m_editor->getSourceDatabase()->getInstance(guid);
	if (!instance)
		return false;

	resolved = instance->getName();
	return true;
}

void PropertiesView::eventPropertyCommand(ui::Event* event)
{
	Ref< ui::custom::BrowsePropertyItem > browseItem = dynamic_type_cast< ui::custom::BrowsePropertyItem* >(event->getItem());
	if (browseItem)
	{
		Ref< db::Instance > instance;

		if (browseItem->getFilterType())
		{
			const Type* filterType = browseItem->getFilterType();

			// Check if filter type is actually a result of a asset; in such case we should
			// browse for the asset and not the final result.
			std::vector< const Type* > assetTypes;
			type_of< Asset >().findAllOf(assetTypes);

			for (std::vector< const Type* >::iterator i = assetTypes.begin(); i != assetTypes.end(); ++i)
			{
				Ref< Asset > asset = dynamic_type_cast< Asset* >((*i)->newInstance());
				if (asset && asset->getOutputType())
				{
					if (is_type_of(*asset->getOutputType(), *filterType))
					{
						filterType = *i;
						break;
					}
				}
			}

			editor::TypeBrowseFilter filter(*filterType);
			instance = m_editor->browseInstance(&filter);
		}
		else
			instance = m_editor->browseInstance();

		if (instance)
		{
			browseItem->setValue(instance->getGuid());

			m_propertyList->apply();

			m_editor->getActiveEditorPage()->propertiesChanged();
		}
	}

	Ref< ui::custom::ObjectPropertyItem > objectItem = dynamic_type_cast< ui::custom::ObjectPropertyItem* >(event->getItem());
	if (objectItem)
	{
		const Type* objectType = objectItem->getObjectType();
		if (!objectType)
			objectType = &type_of< Serializable >();

		objectType = m_editor->browseType(objectType);
		if (objectType)
		{
			Ref< Serializable > object = dynamic_type_cast< Serializable* >(objectType->newInstance());
			if (object)
			{
				objectItem->setObject(object);

				m_propertyList->refresh(objectItem, object);
				m_propertyList->apply();

				m_editor->getActiveEditorPage()->propertiesChanged();
			}
		}
	}
	
	Ref< ui::custom::ArrayPropertyItem > arrayItem = dynamic_type_cast< ui::custom::ArrayPropertyItem* >(event->getItem());
	if (arrayItem)
	{
		if (arrayItem->getElementType())
		{
			const Type* objectType = m_editor->browseType(arrayItem->getElementType());
			if (objectType)
			{
				Ref< Serializable > object = dynamic_type_cast< Serializable* >(objectType->newInstance());
				if (object)
				{
					m_propertyList->addObject(arrayItem, object);
					m_propertyList->apply();
					m_propertyList->refresh();

					m_editor->getActiveEditorPage()->propertiesChanged();
				}
			}
		}
		else	// Non-complex array; just apply and refresh.
		{
			m_propertyList->apply();
			m_propertyList->refresh();
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
			m_editor->getActiveEditorPage()->propertiesChanged();
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
			m_editor->getActiveEditorPage()->propertiesChanged();
		}
		colorDialog.destroy();
	}

	m_propertyList->update();
}

void PropertiesView::eventPropertyChange(ui::Event* event)
{
	m_propertyList->apply();
	m_editor->getActiveEditorPage()->propertiesChanged();
}

	}
}
