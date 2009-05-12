#include "Ui/Custom/PropertyList/AutoPropertyList.h"
#include "Ui/Custom/PropertyList/PropertyItem.h"
#include "Ui/Custom/PropertyList/InspectReflector.h"
#include "Ui/Custom/PropertyList/ApplyReflector.h"

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
	RefList< PropertyItem > children = item->getChildItems();
	for (RefList< PropertyItem >::iterator i = children.begin(); i != children.end(); ++i)
	{
		removeAllChildren(list, *i);
		list->removePropertyItem(item, *i);
	}
	T_ASSERT (item->getChildItems().empty());
}

			}
		
T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.AutoPropertyList", AutoPropertyList, PropertyList)

bool AutoPropertyList::bind(Serializable* object)
{
	removeAllPropertyItems();
	if (!(m_object = object))
		return true;

	InspectReflector reflector(this);
	return reflector.serialize(m_object, m_object->getVersion());
}

bool AutoPropertyList::refresh()
{
	removeAllPropertyItems();
	if (!m_object)
		return true;

	InspectReflector reflector(this);
	return reflector.serialize(m_object, m_object->getVersion());
}

bool AutoPropertyList::refresh(PropertyItem* parent, Serializable* object)
{
	removeAllChildren(this, parent);
	parent->collapse();

	InspectReflector reflector(this, parent);
	return reflector.serialize(object, object->getVersion());
}

bool AutoPropertyList::apply()
{
	if (!m_object)
		return false;

	ApplyReflector reflector(this);
	return reflector.serialize(m_object, m_object->getVersion());
}

bool AutoPropertyList::addObject(PropertyItem* parent, Serializable* object)
{
	T_ASSERT_M (m_object, L"No object currently bound to property list");

	parent->collapse();

	InspectReflector reflector(this, parent);
	return reflector >> Member< Serializable* >(L"item", object);
}

		}
	}
}
