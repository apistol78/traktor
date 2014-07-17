#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyStringSet.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "I18N/Text.h"
#include "Sound/Resound/IGrainData.h"
#include "Sound/Editor/Resound/GrainProperties.h"
#include "Ui/Custom/PropertyList/ArrayPropertyItem.h"
#include "Ui/Custom/PropertyList/BrowsePropertyItem.h"
#include "Ui/Custom/PropertyList/ObjectPropertyItem.h"
#include "Ui/Custom/PropertyList/PropertyCommandEvent.h"
#include "Ui/Custom/PropertyList/PropertyContentChangeEvent.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.GrainProperties", GrainProperties, ui::EventSubject)

GrainProperties::GrainProperties(editor::IEditor* editor)
:	m_editor(editor)
{
}

bool GrainProperties::create(ui::Widget* parent)
{
	m_grainPropertyList = new ui::custom::AutoPropertyList();
	m_grainPropertyList->create(parent, ui::WsDoubleBuffer | ui::custom::AutoPropertyList::WsColumnHeader, this);
	m_grainPropertyList->addEventHandler< ui::custom::PropertyCommandEvent >(this, &GrainProperties::eventPropertyCommand);
	m_grainPropertyList->addEventHandler< ui::custom::PropertyContentChangeEvent >(this, &GrainProperties::eventPropertyChange);
	m_grainPropertyList->setSeparator(150);
	m_grainPropertyList->setColumnName(0, i18n::Text(L"PROPERTY_COLUMN_NAME"));
	m_grainPropertyList->setColumnName(1, i18n::Text(L"PROPERTY_COLUMN_VALUE"));
	return true;
}

void GrainProperties::destroy()
{
	safeDestroy(m_grainPropertyList);
	m_grain = 0;
}

void GrainProperties::set(IGrainData* grain)
{
	// Capture state of current grain.
	if (m_grain)
		m_states[&type_of(m_grain)] = m_grainPropertyList->captureState();

	m_grainPropertyList->bind(grain);

	// Restore state of last property object of same type.
	if (grain)
	{
		std::map< const TypeInfo*, Ref< ui::HierarchicalState > >::iterator i = m_states.find(&type_of(grain));
		if (i != m_states.end())
			m_grainPropertyList->applyState(i->second);
	}

	m_grain = grain;
}

void GrainProperties::reset()
{
	set(m_grain);
}

bool GrainProperties::handleCommand(const ui::Command& command)
{
	if (command == L"Editor.Copy")
		return m_grainPropertyList->copy();
	else if (command == L"Editor.Paste")
	{
		if (m_grainPropertyList->paste())
		{
			m_grainPropertyList->apply();
			return true;
		}
		else
			return false;
	}
	else
		return false;
}

bool GrainProperties::resolvePropertyGuid(const Guid& guid, std::wstring& resolved) const
{
	Ref< db::Instance > instance = m_editor->getSourceDatabase()->getInstance(guid);
	if (!instance)
		return false;

	resolved = instance->getName();
	return true;
}

void GrainProperties::eventPropertyCommand(ui::custom::PropertyCommandEvent* event)
{
	const ui::Command& cmd = event->getCommand();

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
					m_grainPropertyList->apply();

					ui::ContentChangeEvent contentChangeEvent(this);
					raiseEvent(&contentChangeEvent);
				}
			}
			else
			{
				browseItem->setValue(Guid());
				m_grainPropertyList->apply();

				ui::ContentChangeEvent contentChangeEvent(this);
				raiseEvent(&contentChangeEvent);
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

					m_grainPropertyList->refresh(objectItem, object);
					m_grainPropertyList->apply();

					ui::ContentChangeEvent contentChangeEvent(this);
					raiseEvent(&contentChangeEvent);
				}
			}
		}
		else
		{
			if (ui::custom::ArrayPropertyItem* parentArrayItem = dynamic_type_cast< ui::custom::ArrayPropertyItem* >(objectItem->getParentItem()))
				m_grainPropertyList->removePropertyItem(parentArrayItem, objectItem);
			else
				objectItem->setObject(0);

			m_grainPropertyList->refresh(objectItem, 0);
			m_grainPropertyList->apply();

			ui::ContentChangeEvent contentChangeEvent(this);
			raiseEvent(&contentChangeEvent);
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
					m_grainPropertyList->addObject(arrayItem, object);
					m_grainPropertyList->apply();
					m_grainPropertyList->refresh();

					ui::ContentChangeEvent contentChangeEvent(this);
					raiseEvent(&contentChangeEvent);
				}
			}
		}
		else	// Non-complex array; just apply and refresh.
		{
			m_grainPropertyList->apply();
			m_grainPropertyList->refresh();

			ui::ContentChangeEvent contentChangeEvent(this);
			raiseEvent(&contentChangeEvent);
		}
	}

	m_grainPropertyList->update();
}

void GrainProperties::eventPropertyChange(ui::custom::PropertyContentChangeEvent* event)
{
	m_grainPropertyList->apply();

	ui::ContentChangeEvent contentChangeEvent(this);
	raiseEvent(&contentChangeEvent);
}

	}
}
