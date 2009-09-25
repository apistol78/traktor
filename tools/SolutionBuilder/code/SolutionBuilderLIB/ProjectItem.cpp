#include <Core/Serialization/Serializer.h>
#include <Core/Serialization/MemberRef.h>
#include "ProjectItem.h"

using namespace traktor;

T_IMPLEMENT_RTTI_CLASS(L"ProjectItem", ProjectItem, Serializable)

void ProjectItem::addItem(ProjectItem* item)
{
	m_items.push_back(item);
}

void ProjectItem::removeItem(ProjectItem* item)
{
	m_items.remove(item);
}

const RefList< ProjectItem >& ProjectItem::getItems() const
{
	return m_items;
}

bool ProjectItem::serialize(Serializer& s)
{
	return s >> MemberRefList< ProjectItem >(L"items", m_items);
}
