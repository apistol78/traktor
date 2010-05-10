#include "Core/Misc/SafeDestroy.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "Editor/IEditorPage.h"
#include "Editor/ITypedAsset.h"
#include "Editor/TypeBrowseFilter.h"
#include "Editor/App/PropertiesView.h"
#include "Editor/App/TextEditorDialog.h"
#include "I18N/Text.h"
#include "Ui/FileDialog.h"
#include "Ui/FloodLayout.h"
#include "Ui/MethodHandler.h"
#include "Ui/Events/CommandEvent.h"
#include "Ui/Custom/PropertyList/FilePropertyItem.h"
#include "Ui/Custom/PropertyList/BrowsePropertyItem.h"
#include "Ui/Custom/PropertyList/ObjectPropertyItem.h"
#include "Ui/Custom/PropertyList/ArrayPropertyItem.h"
#include "Ui/Custom/PropertyList/TextPropertyItem.h"
#include "Ui/Custom/PropertyList/ColorPropertyItem.h"
#include "Ui/Custom/ColorPicker/ColorDialog.h"

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
	if (!ui::Container::create(parent, ui::WsNone, new ui::FloodLayout()))
		return false;

	m_propertyList = new ui::custom::AutoPropertyList();
	m_propertyList->create(this, ui::WsDoubleBuffer, this);
	m_propertyList->addCommandEventHandler(ui::createMethodHandler(this, &PropertiesView::eventPropertyCommand));
	m_propertyList->addChangeEventHandler(ui::createMethodHandler(this, &PropertiesView::eventPropertyChange));

	return true;
}

void PropertiesView::destroy()
{
	safeDestroy(m_propertyList);
	ui::Container::destroy();
}

void PropertiesView::setPropertyObject(ISerializable* object, ISerializable* outer)
{
	// Capture state of current property object.
	if (m_propertyObject)
		m_states[&type_of(m_propertyObject)] = m_propertyList->captureState();

	m_propertyList->hide();
	
	// Bind property object.
	m_propertyList->bind(object, outer);

	// Restore state of last property object of same type.
	if (object)
	{
		std::map< const TypeInfo*, Ref< ui::HierarchicalState > >::iterator i = m_states.find(&type_of(object));
		if (i != m_states.end())
			m_propertyList->applyState(i->second);
	}
	
	m_propertyList->show();

	m_propertyObject = object;
}

Ref< ISerializable > PropertiesView::getPropertyObject()
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
	const ui::CommandEvent* cmdEvent = checked_type_cast< const ui::CommandEvent* >(event);
	const ui::Command& cmd = cmdEvent->getCommand();

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

					// Check if filter type is actually a result of a asset; in such case we should
					// browse for the asset and not the final result.
					TypeInfoSet filterTypes;

					std::vector< const TypeInfo* > assetTypes;
					type_of< ITypedAsset >().findAllOf(assetTypes);
					for (std::vector< const TypeInfo* >::iterator i = assetTypes.begin(); i != assetTypes.end(); ++i)
					{
						Ref< ITypedAsset > asset = dynamic_type_cast< ITypedAsset* >((*i)->createInstance());
						if (asset && asset->getOutputType())
						{
							if (is_type_of(*asset->getOutputType(), *filterType))
								filterTypes.insert(*i);
						}
					}

					if (filterTypes.empty())
						filterTypes.insert(filterType);

					editor::TypeBrowseFilter filter(filterTypes);
					instance = m_editor->browseInstance(&filter);
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

void PropertiesView::eventPropertyChange(ui::Event* event)
{
	m_propertyList->apply();
	m_editor->getActiveEditorPage()->handleCommand(ui::Command(L"Editor.PropertiesChanged"));
}

	}
}
