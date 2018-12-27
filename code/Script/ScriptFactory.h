/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_script_ScriptFactory_H
#define traktor_script_ScriptFactory_H

#include "Resource/IResourceFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCRIPT_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace script
	{

class IScriptContext;

/*! \brief Script class factory.
 * \ingroup Script
 */
class T_DLLCLASS ScriptFactory : public resource::IResourceFactory
{
	T_RTTI_CLASS;

public:
	ScriptFactory(IScriptContext* scriptContext);

	virtual const TypeInfoSet getResourceTypes() const override final;

	virtual const TypeInfoSet getProductTypes(const TypeInfo& resourceType) const override final;

	virtual bool isCacheable(const TypeInfo& productType) const override final;

	virtual Ref< Object > create(resource::IResourceManager* resourceManager, const db::Database* database, const db::Instance* instance, const TypeInfo& productType, const Object* current) const override final;

private:
	Ref< IScriptContext > m_scriptContext;
};

	}
}

#endif	// traktor_script_ScriptFactory_H
