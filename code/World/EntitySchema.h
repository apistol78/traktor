#ifndef traktor_world_EntitySchema_H
#define traktor_world_EntitySchema_H

#include <map>
#include "World/IEntitySchema.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

/*! \brief Entity schema implementation.
 * \ingroup World
 */
class T_DLLCLASS EntitySchema : public IEntitySchema
{
	T_RTTI_CLASS;

public:
	virtual void insertEntity(const Entity* parentEntity, const std::wstring& name, Entity* entity) T_OVERRIDE T_FINAL;

	virtual Entity* getEntity(uint32_t index) const T_OVERRIDE T_FINAL;

	virtual Entity* getEntity(const std::wstring& name, uint32_t index = 0) const T_OVERRIDE T_FINAL;

	virtual Entity* getEntity(const TypeInfo& entityType, uint32_t index = 0) const T_OVERRIDE T_FINAL;

	virtual Entity* getEntity(const std::wstring& name, const TypeInfo& entityType, uint32_t index = 0) const T_OVERRIDE T_FINAL;

	virtual Entity* getChildEntity(const Entity* parentEntity, uint32_t index = 0) const T_OVERRIDE T_FINAL;

	virtual Entity* getChildEntity(const Entity* parentEntity, const std::wstring& name, uint32_t index = 0) const T_OVERRIDE T_FINAL;

	virtual Entity* getChildEntity(const Entity* parentEntity, const TypeInfo& entityType, uint32_t index = 0) const T_OVERRIDE T_FINAL;

	virtual Entity* getChildEntity(const Entity* parentEntity, const std::wstring& name, const TypeInfo& entityType, uint32_t index = 0) const T_OVERRIDE T_FINAL;

	virtual uint32_t getEntities(RefArray< Entity >& outEntities) const T_OVERRIDE T_FINAL;

	virtual uint32_t getEntities(const std::wstring& name, RefArray< Entity >& outEntities) const T_OVERRIDE T_FINAL;

	virtual uint32_t getEntities(const TypeInfo& entityType, RefArray< Entity >& outEntities) const T_OVERRIDE T_FINAL;

	virtual uint32_t getEntities(const std::wstring& name, const TypeInfo& entityType, RefArray< Entity >& outEntities) const T_OVERRIDE T_FINAL;

	virtual uint32_t getChildEntities(const Entity* parentEntity, RefArray< Entity >& outEntities) const T_OVERRIDE T_FINAL;

	virtual uint32_t getChildEntities(const Entity* parentEntity, const std::wstring& name, RefArray< Entity >& outEntities) const T_OVERRIDE T_FINAL;

	virtual uint32_t getChildEntities(const Entity* parentEntity, const TypeInfo& entityType, RefArray< Entity >& outEntities) const T_OVERRIDE T_FINAL;

	virtual uint32_t getChildEntities(const Entity* parentEntity, const std::wstring& name, const TypeInfo& entityType, RefArray< Entity >& outEntities) const T_OVERRIDE T_FINAL;

private:
	struct EntityInfo
	{
		const Entity* parent;
		Entity* entity;
	};

	struct Indices
	{
		std::vector< uint32_t > children;
		std::map< std::wstring, std::vector< uint32_t > > name;
		std::map< const TypeInfo*, std::vector< uint32_t > > type;
	};

	std::vector< EntityInfo > m_entityInfo;

	Indices m_global;
	std::map< const Entity*, Indices > m_children;
};

	}
}

#endif	// traktor_world_EntitySchema_H
