#include <Core/Serialization/Serializer.h>
#include <Core/Serialization/MemberRef.h>
#include "ProjectItem.h"

T_IMPLEMENT_RTTI_CLASS(L"ProjectItem", ProjectItem, traktor::Serializable)

void ProjectItem::addItem(ProjectItem* item)
{
	m_items.push_back(item);
}

void ProjectItem::removeItem(ProjectItem* item)
{
	m_items.remove(item);
}

traktor::RefList< ProjectItem >& ProjectItem::getItems()
{
	return m_items;
}

bool ProjectItem::serialize(traktor::Serializer& s)
{
	return s >> traktor::MemberRefList< ProjectItem >(L"items", m_items);
}
