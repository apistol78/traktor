/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_scene_LayerEntityFactory_H
#define traktor_scene_LayerEntityFactory_H

#include "World/IEntityFactory.h"

namespace traktor
{
	namespace scene
	{

/*! \brief Layer entity factory.
 * \ingroup Scene
 */
class LayerEntityFactory : public world::IEntityFactory
{
	T_RTTI_CLASS;
	
public:
	virtual const TypeInfoSet getEntityTypes() const override final;

	virtual const TypeInfoSet getEntityEventTypes() const override final;

	virtual const TypeInfoSet getEntityComponentTypes() const override final;

	virtual Ref< world::Entity > createEntity(const world::IEntityBuilder* builder, const world::EntityData& entityData) const override final;

	virtual Ref< world::IEntityEvent > createEntityEvent(const world::IEntityBuilder* builder, const world::IEntityEventData& entityEventData) const override final;

	virtual Ref< world::IEntityComponent > createEntityComponent(const world::IEntityBuilder* builder, const world::IEntityComponentData& entityComponentData) const override final;
};
	
	}
}

#endif	// traktor_scene_LayerEntityFactory_H
