/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/Custom/PropertyList/ApplyReflector.h"
#include "Ui/Custom/PropertyList/AutoPropertyList.h"
#include "Ui/Custom/PropertyList/InspectReflector.h"
#include "Ui/Custom/PropertyList/ObjectPropertyItem.h"
#include "Ui/Custom/PropertyList/PropertyItem.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{
			namespace
			{

void removeAllChildren(PropertyList* list, PropertyItem* item)
{
	RefArray< PropertyItem > children = item->getChildItems();
	for (RefArray< PropertyItem >::iterator i = children.begin(); i != children.end(); ++i)
	{
		removeAllChildren(list, *i);
		list->removePropertyItem(item, *i);
	}
	T_ASSERT (item->getChildItems().empty());
}

			}
		
T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.AutoPropertyList", AutoPropertyList, PropertyList)

bool AutoPropertyList::bind(ISerializable* object)
{
	removeAllPropertyItems();
	if (!(m_object = object))
	{
		update();
		return true;
	}

	InspectReflector reflector(this);
	reflector.serialize(m_object);

	update();
	return true;
}

bool AutoPropertyList::refresh()
{
	Ref< HierarchicalState > state = captureState();

	removeAllPropertyItems();
	if (!m_object)
		return true;

	InspectReflector reflector(this);
	reflector.serialize(m_object);

	applyState(state);
	update();
	return true;
}

bool AutoPropertyList::refresh(PropertyItem* parent, ISerializable* object)
{
	Ref< HierarchicalState > state = captureState();

	removeAllChildren(this, parent);
	parent->collapse();

	if (object)
	{
		InspectReflector reflector(this, parent);
		reflector.serialize(object);
	}

	applyState(state);
	update();
	return true;
}

bool AutoPropertyList::apply()
{
	if (!m_object)
		return false;

	ApplyReflector reflector(this);
	reflector.serialize(m_object);

	return true;
}

bool AutoPropertyList::addObject(PropertyItem* parent, ISerializable* object)
{
	T_ASSERT_M (m_object, L"No object currently bound to property list");

	Ref< HierarchicalState > state = captureState();
	parent->collapse();

	InspectReflector reflector(this, parent);

	Ref< ISerializable > mutableObject = object;
	reflector >> Member< ISerializable* >(L"item", mutableObject);

	applyState(state);
	update();
	return true;
}

bool AutoPropertyList::paste()
{
	RefArray< PropertyItem > selectedItems;
	getPropertyItems(selectedItems, GfDescendants | GfSelectedOnly);
	if (selectedItems.size() == 1 && selectedItems[0]->paste())
	{
		if (ObjectPropertyItem* objectItem = dynamic_type_cast< ObjectPropertyItem* >(selectedItems[0]))
			refresh(objectItem, checked_type_cast< ISerializable*, true >(objectItem->getObject()));
		else
			update();

		return true;
	}
	else
		return false;
}

		}
	}
}
