/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/PropertyList/ApplyReflector.h"
#include "Ui/PropertyList/AutoPropertyList.h"
#include "Ui/PropertyList/InspectReflector.h"
#include "Ui/PropertyList/ObjectPropertyItem.h"
#include "Ui/PropertyList/PropertyItem.h"

namespace traktor::ui
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
	T_ASSERT(item->getChildItems().empty());
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.AutoPropertyList", AutoPropertyList, PropertyList)

bool AutoPropertyList::bind(ISerializable* object)
{
	removeAllPropertyItems();
	if ((m_object = object) != nullptr)
	{
		InspectReflector reflector(this);
		reflector.serialize(m_object);
	}
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
