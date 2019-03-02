#include "World/Entity.h"
#include "World/EntitySchema.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.EntitySchema", EntitySchema, IEntitySchema)

void EntitySchema::insertEntity(const Entity* parentEntity, const std::wstring& name, Entity* entity)
{
	if (!entity)
		return;

	EntityInfo info;
	info.parent = parentEntity;
	info.entity = entity;

	uint32_t index = uint32_t(m_entityInfo.size());
	m_entityInfo.push_back(info);

	m_global.name[name].push_back(index);
	for (const TypeInfo* type = &type_of(entity); type != &type_of< Object >(); type = type->getSuper())
		m_global.type[type].push_back(index);

	Indices& indices = m_children[parentEntity];
	indices.children.push_back(index);

	indices.name[name].push_back(index);
	for (const TypeInfo* type = &type_of(entity); type != &type_of< Object >(); type = type->getSuper())
		indices.type[type].push_back(index);
}

Entity* EntitySchema::getEntity(uint32_t index) const
{
	if (index < m_entityInfo.size())
		return m_entityInfo[index].entity;
	else
		return 0;
}

Entity* EntitySchema::getEntity(const std::wstring& name, uint32_t index) const
{
	std::map< std::wstring, std::vector< uint32_t > >::const_iterator i = m_global.name.find(name);
	if (i == m_global.name.end())
		return 0;

	if (index >= i->second.size())
		return 0;

	return m_entityInfo[i->second[index]].entity;
}

Entity* EntitySchema::getEntity(const TypeInfo& entityType, uint32_t index) const
{
	std::map< const TypeInfo*, std::vector< uint32_t > >::const_iterator i = m_global.type.find(&entityType);
	if (i == m_global.type.end())
		return 0;

	if (index >= i->second.size())
		return 0;

	return m_entityInfo[i->second[index]].entity;
}

Entity* EntitySchema::getEntity(const std::wstring& name, const TypeInfo& entityType, uint32_t index) const
{
	std::map< std::wstring, std::vector< uint32_t > >::const_iterator i = m_global.name.find(name);
	if (i == m_global.name.end())
		return 0;

	for (std::vector< uint32_t >::const_iterator j = i->second.begin(); j != i->second.end(); ++j)
	{
		if (!is_type_of(entityType, type_of(m_entityInfo[*j].entity)))
			continue;

		if (index-- > 0)
			continue;

		return m_entityInfo[*j].entity;
	}

	return 0;
}

Entity* EntitySchema::getChildEntity(const Entity* parentEntity, uint32_t index) const
{
	std::map< const Entity*, Indices >::const_iterator i = m_children.find(parentEntity);
	if (i == m_children.end())
		return 0;

	const Indices& local = i->second;
	if (index >= local.children.size())
		return 0;

	return m_entityInfo[local.children[index]].entity;
}

Entity* EntitySchema::getChildEntity(const Entity* parentEntity, const std::wstring& name, uint32_t index) const
{
	std::map< const Entity*, Indices >::const_iterator i = m_children.find(parentEntity);
	if (i == m_children.end())
		return 0;

	const Indices& local = i->second;

	std::map< std::wstring, std::vector< uint32_t > >::const_iterator j = local.name.find(name);
	if (j == local.name.end())
		return 0;

	if (index >= j->second.size())
		return 0;

	return m_entityInfo[j->second[index]].entity;
}

Entity* EntitySchema::getChildEntity(const Entity* parentEntity, const TypeInfo& entityType, uint32_t index) const
{
	std::map< const Entity*, Indices >::const_iterator i = m_children.find(parentEntity);
	if (i == m_children.end())
		return 0;

	const Indices& local = i->second;

	std::map< const TypeInfo*, std::vector< uint32_t > >::const_iterator j = local.type.find(&entityType);
	if (j == local.type.end())
		return 0;

	if (index >= j->second.size())
		return 0;

	return m_entityInfo[j->second[index]].entity;
}

Entity* EntitySchema::getChildEntity(const Entity* parentEntity, const std::wstring& name, const TypeInfo& entityType, uint32_t index) const
{
	std::map< const Entity*, Indices >::const_iterator i = m_children.find(parentEntity);
	if (i == m_children.end())
		return 0;

	const Indices& local = i->second;

	std::map< std::wstring, std::vector< uint32_t > >::const_iterator j = local.name.find(name);
	if (j == local.name.end())
		return 0;

	for (std::vector< uint32_t >::const_iterator k = j->second.begin(); k != j->second.end(); ++k)
	{
		if (!is_type_of(entityType, type_of(m_entityInfo[*k].entity)))
			continue;

		if (index-- > 0)
			continue;

		return m_entityInfo[*k].entity;
	}

	return 0;
}

uint32_t EntitySchema::getEntities(RefArray< Entity >& outEntities) const
{
	outEntities.resize(0);
	outEntities.reserve(m_entityInfo.size());

	for (std::vector< EntityInfo >::const_iterator i = m_entityInfo.begin(); i != m_entityInfo.end(); ++i)
		outEntities.push_back(i->entity);

	return uint32_t(m_entityInfo.size());
}

uint32_t EntitySchema::getEntities(const std::wstring& name, RefArray< Entity >& outEntities) const
{
	outEntities.resize(0);

	std::map< std::wstring, std::vector< uint32_t > >::const_iterator i = m_global.name.find(name);
	if (i == m_global.name.end())
		return 0;

	outEntities.reserve(i->second.size());
	for (std::vector< uint32_t >::const_iterator j = i->second.begin(); j != i->second.end(); ++j)
		outEntities.push_back(m_entityInfo[*j].entity);

	return uint32_t(i->second.size());
}

uint32_t EntitySchema::getEntities(const TypeInfo& entityType, RefArray< Entity >& outEntities) const
{
	outEntities.resize(0);

	std::map< const TypeInfo*, std::vector< uint32_t > >::const_iterator i = m_global.type.find(&entityType);
	if (i == m_global.type.end())
		return 0;

	outEntities.reserve(i->second.size());
	for (std::vector< uint32_t >::const_iterator j = i->second.begin(); j != i->second.end(); ++j)
		outEntities.push_back(m_entityInfo[*j].entity);

	return uint32_t(i->second.size());
}

uint32_t EntitySchema::getEntities(const std::wstring& name, const TypeInfo& entityType, RefArray< Entity >& outEntities) const
{
	outEntities.resize(0);

	std::map< std::wstring, std::vector< uint32_t > >::const_iterator i = m_global.name.find(name);
	if (i == m_global.name.end())
		return 0;

	outEntities.reserve(i->second.size());
	for (std::vector< uint32_t >::const_iterator j = i->second.begin(); j != i->second.end(); ++j)
	{
		if (is_type_of(entityType, type_of(m_entityInfo[*j].entity)))
			outEntities.push_back(m_entityInfo[*j].entity);
	}

	return uint32_t(outEntities.size());
}

uint32_t EntitySchema::getChildEntities(const Entity* parentEntity, RefArray< Entity >& outEntities) const
{
	outEntities.resize(0);

	std::map< const Entity*, Indices >::const_iterator i = m_children.find(parentEntity);
	if (i == m_children.end())
		return 0;

	const Indices& local = i->second;

	outEntities.reserve(local.children.size());
	for (std::vector< uint32_t >::const_iterator j = local.children.begin(); j != local.children.end(); ++j)
		outEntities.push_back(m_entityInfo[*j].entity);

	return uint32_t(local.children.size());
}

uint32_t EntitySchema::getChildEntities(const Entity* parentEntity, const std::wstring& name, RefArray< Entity >& outEntities) const
{
	outEntities.resize(0);

	std::map< const Entity*, Indices >::const_iterator i = m_children.find(parentEntity);
	if (i == m_children.end())
		return 0;

	const Indices& local = i->second;

	std::map< std::wstring, std::vector< uint32_t > >::const_iterator j = local.name.find(name);
	if (j == local.name.end())
		return 0;

	outEntities.reserve(j->second.size());
	for (std::vector< uint32_t >::const_iterator k = j->second.begin(); k != j->second.end(); ++k)
		outEntities.push_back(m_entityInfo[*k].entity);

	return uint32_t(j->second.size());
}

uint32_t EntitySchema::getChildEntities(const Entity* parentEntity, const TypeInfo& entityType, RefArray< Entity >& outEntities) const
{
	outEntities.resize(0);

	std::map< const Entity*, Indices >::const_iterator i = m_children.find(parentEntity);
	if (i == m_children.end())
		return 0;

	const Indices& local = i->second;

	std::map< const TypeInfo*, std::vector< uint32_t > >::const_iterator j = local.type.find(&entityType);
	if (j == local.type.end())
		return 0;

	outEntities.reserve(j->second.size());
	for (std::vector< uint32_t >::const_iterator k = j->second.begin(); k != j->second.end(); ++k)
		outEntities.push_back(m_entityInfo[*k].entity);

	return uint32_t(j->second.size());
}

uint32_t EntitySchema::getChildEntities(const Entity* parentEntity, const std::wstring& name, const TypeInfo& entityType, RefArray< Entity >& outEntities) const
{
	outEntities.resize(0);

	std::map< const Entity*, Indices >::const_iterator i = m_children.find(parentEntity);
	if (i == m_children.end())
		return 0;

	const Indices& local = i->second;

	std::map< std::wstring, std::vector< uint32_t > >::const_iterator j = local.name.find(name);
	if (j == local.name.end())
		return 0;

	outEntities.reserve(j->second.size());
	for (std::vector< uint32_t >::const_iterator k = j->second.begin(); k != j->second.end(); ++k)
	{
		if (is_type_of(entityType, type_of(m_entityInfo[*k].entity)))
			outEntities.push_back(m_entityInfo[*k].entity);
	}

	return uint32_t(outEntities.size());
}

	}
}
