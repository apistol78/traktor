/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_spark_CharacterResourceFactory_H
#define traktor_spark_CharacterResourceFactory_H

#include "Resource/IResourceFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace spark
	{

/*! \brief Character resource factory.
 * \ingroup Spark
 */
class T_DLLCLASS CharacterResourceFactory : public resource::IResourceFactory
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfoSet getResourceTypes() const T_FINAL;

	virtual const TypeInfoSet getProductTypes(const TypeInfo& resourceType) const T_OVERRIDE T_FINAL;

	virtual bool isCacheable(const TypeInfo& productType) const T_FINAL;

	virtual Ref< Object > create(resource::IResourceManager* resourceManager, const db::Database* database, const db::Instance* instance, const TypeInfo& productType, const Object* current) const T_FINAL;
};

	}
}

#endif	// traktor_spark_CharacterResourceFactory_H
