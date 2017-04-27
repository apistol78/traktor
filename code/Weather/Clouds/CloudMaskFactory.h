/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_weather_CloudMaskFactory_H
#define traktor_weather_CloudMaskFactory_H

#include "Resource/IResourceFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WEATHER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace weather
	{

class T_DLLCLASS CloudMaskFactory : public resource::IResourceFactory
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfoSet getResourceTypes() const T_OVERRIDE T_FINAL;

	virtual const TypeInfoSet getProductTypes(const TypeInfo& resourceType) const T_OVERRIDE T_FINAL;

	virtual bool isCacheable(const TypeInfo& productType) const T_OVERRIDE T_FINAL;

	virtual Ref< Object > create(resource::IResourceManager* resourceManager, const db::Database* database, const db::Instance* instance, const TypeInfo& productType, const Object* current) const T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_weather_CloudMaskFactory_H
