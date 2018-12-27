/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_world_EntityEventResourceFactory_H
#define traktor_world_EntityEventResourceFactory_H

#include "Resource/IResourceFactory.h"

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

class IEntityBuilder;

/*! \brief
 * \ingroup World
 */
class T_DLLCLASS EntityEventResourceFactory : public resource::IResourceFactory
{
	T_RTTI_CLASS;

public:
	EntityEventResourceFactory(const IEntityBuilder* entityBuilder);

	virtual const TypeInfoSet getResourceTypes() const override final;

	virtual const TypeInfoSet getProductTypes(const TypeInfo& resourceType) const override final;

	virtual bool isCacheable(const TypeInfo& productType) const override final;

	virtual Ref< Object > create(resource::IResourceManager* resourceManager, const db::Database* database, const db::Instance* instance, const TypeInfo& productType, const Object* current) const override final;

private:
	Ref< const IEntityBuilder > m_entityBuilder;
};

	}
}

#endif	// traktor_world_EntityEventResourceFactory_H
