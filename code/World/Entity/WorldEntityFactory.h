/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_world_WorldEntityFactory_H
#define traktor_world_WorldEntityFactory_H

#include "World/IEntityFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace resource
	{

class IResourceManager;

	}

	namespace world
	{

/*! \brief World entity factory.
 * \ingroup World
 */
class T_DLLCLASS WorldEntityFactory : public IEntityFactory
{
	T_RTTI_CLASS;
	
public:
	WorldEntityFactory(resource::IResourceManager* resourceManager, bool editor);

	virtual const TypeInfoSet getEntityTypes() const T_OVERRIDE T_FINAL;

	virtual const TypeInfoSet getEntityEventTypes() const T_OVERRIDE T_FINAL;

	virtual const TypeInfoSet getEntityComponentTypes() const T_OVERRIDE T_FINAL;

	virtual Ref< Entity > createEntity(const IEntityBuilder* builder, const EntityData& entityData) const T_OVERRIDE T_FINAL;

	virtual Ref< IEntityEvent > createEntityEvent(const IEntityBuilder* builder, const IEntityEventData& entityEventData) const T_OVERRIDE T_FINAL;

	virtual Ref< IEntityComponent > createEntityComponent(const world::IEntityBuilder* builder, const IEntityComponentData& entityComponentData) const T_OVERRIDE T_FINAL;

private:
	mutable Ref< resource::IResourceManager > m_resourceManager;
	bool m_editor;
};
	
	}
}

#endif	// traktor_world_WorldEntityFactory_H
