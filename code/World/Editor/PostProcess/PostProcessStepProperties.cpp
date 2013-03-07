#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyStringSet.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "I18N/Text.h"
#include "Ui/MethodHandler.h"
#include "Ui/Events/CommandEvent.h"
#include "Ui/Custom/PropertyList/ArrayPropertyItem.h"
#include "Ui/Custom/PropertyList/BrowsePropertyItem.h"
#include "Ui/Custom/PropertyList/ObjectPropertyItem.h"
#include "World/PostProcess/PostProcessStep.h"
#include "World/Editor/PostProcess/PostProcessStepProperties.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.PostProcessStepProperties", PostProcessStepProperties, ui::EventSubject)

PostProcessStepProperties::PostProcessStepProperties(editor::IEditor* editor)
:	m_editor(editor)
{
}

bool PostProcessStepProperties::create(ui::Widget* parent)
{
	m_propertyList = new ui::custom::AutoPropertyList();
	m_propertyList->create(parent, ui::WsDoubleBuffer | ui::custom::AutoPropertyList::WsColumnHeader, this);
	m_propertyList->addCommandEventHandler(ui::createMethodHandler(this, &PostProcessStepProperties::eventPropertyCommand));
	m_propertyList->addChangeEventHandler(ui::createMethodHandler(this, &PostProcessStepProperties::eventPropertyChange));
	m_propertyList->setSeparator(200);
	m_propertyList->setColumnName(0, i18n::Text(L"PROPERTY_COLUMN_NAME"));
	m_propertyList->setColumnName(1, i18n::Text(L"PROPERTY_COLUMN_VALUE"));
	return true;
}

void PostProcessStepProperties::destroy()
{
	safeDestroy(m_propertyList);
}

void PostProcessStepProperties::set(PostProcessStep* step)
{
	m_propertyList->bind(step);
}

bool PostProcessStepProperties::resolvePropertyGuid(const Guid& guid, std::wstring& resolved) const
{
	Ref< db::Instance > instance = m_editor->getSourceDatabase()->getInstance(guid);
	if (!instance)
		return false;

	resolved = instance->getName();
	return true;
}

void PostProcessStepProperties::eventPropertyCommand(ui::Event* event)
{
	const ui::CommandEvent* cmdEvent = checked_type_cast< const ui::CommandEvent* >(event);
	const ui::Command& cmd = cmdEvent->getCommand();

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
					raiseEvent(ui::EiUser + 1, 0);
				}
			}
			else
			{
				browseItem->setValue(Guid());
				m_propertyList->apply();
				raiseEvent(ui::EiUser + 1, 0);
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

					raiseEvent(ui::EiUser + 1, 0);
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

			raiseEvent(ui::EiUser + 1, 0);
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
					raiseEvent(ui::EiUser + 1, 0);
				}
			}
		}
		else	// Non-complex array; just apply and refresh.
		{
			m_propertyList->apply();
			m_propertyList->refresh();
			raiseEvent(ui::EiUser + 1, 0);
		}
	}

	m_propertyList->update();
}

void PostProcessStepProperties::eventPropertyChange(ui::Event* event)
{
	raiseEvent(ui::EiUser + 1, 0);
	m_propertyList->apply();
}

	}
}
