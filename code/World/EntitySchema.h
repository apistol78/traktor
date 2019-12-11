#pragma once

#include "Core/Containers/SmallMap.h"
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

/*! Entity schema implementation.
 * \ingroup World
 */
class T_DLLCLASS EntitySchema : public IEntitySchema
{
	T_RTTI_CLASS;

public:
	virtual void insertEntity(const Entity* parentEntity, const std::wstring& name, Entity* entity) override final;

	virtual Entity* getEntity(uint32_t index) const override final;

	virtual Entity* getEntity(const std::wstring& name, uint32_t index = 0) const override final;

	virtual Entity* getEntity(const TypeInfo& entityType, uint32_t index = 0) const override final;

	virtual Entity* getEntity(const std::wstring& name, const TypeInfo& entityType, uint32_t index = 0) const override final;

	virtual Entity* getChildEntity(const Entity* parentEntity, uint32_t index = 0) const override final;

	virtual Entity* getChildEntity(const Entity* parentEntity, const std::wstring& name, uint32_t index = 0) const override final;

	virtual Entity* getChildEntity(const Entity* parentEntity, const TypeInfo& entityType, uint32_t index = 0) const override final;

	virtual Entity* getChildEntity(const Entity* parentEntity, const std::wstring& name, const TypeInfo& entityType, uint32_t index = 0) const override final;

	virtual uint32_t getEntities(RefArray< Entity >& outEntities) const override final;

	virtual uint32_t getEntities(const std::wstring& name, RefArray< Entity >& outEntities) const override final;

	virtual uint32_t getEntities(const TypeInfo& entityType, RefArray< Entity >& outEntities) const override final;

	virtual uint32_t getEntities(const std::wstring& name, const TypeInfo& entityType, RefArray< Entity >& outEntities) const override final;

	virtual uint32_t getChildEntities(const Entity* parentEntity, RefArray< Entity >& outEntities) const override final;

	virtual uint32_t getChildEntities(const Entity* parentEntity, const std::wstring& name, RefArray< Entity >& outEntities) const override final;

	virtual uint32_t getChildEntities(const Entity* parentEntity, const TypeInfo& entityType, RefArray< Entity >& outEntities) const override final;

	virtual uint32_t getChildEntities(const Entity* parentEntity, const std::wstring& name, const TypeInfo& entityType, RefArray< Entity >& outEntities) const override final;

private:
	struct EntityInfo
	{
		const Entity* parent;
		Entity* entity;
	};

	struct Indices
	{
		AlignedVector< uint32_t > children;
		SmallMap< std::wstring, AlignedVector< uint32_t > > name;
		SmallMap< const TypeInfo*, AlignedVector< uint32_t > > type;
	};

	AlignedVector< EntityInfo > m_entityInfo;
	Indices m_global;
	SmallMap< const Entity*, Indices > m_children;
};

	}
}

