#ifndef traktor_script_ScriptModuleFactory_H
#define traktor_script_ScriptModuleFactory_H

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
	namespace db
	{

class Database;

	}

	namespace script
	{

class IScriptManager;

/*! \brief Script module factory.
 * \ingroup Script
 */
class T_DLLCLASS ScriptModuleFactory : public resource::IResourceFactory
{
	T_RTTI_CLASS;

public:
	ScriptModuleFactory(db::Database* database, IScriptManager* scriptManager);

	virtual const TypeInfoSet getResourceTypes() const T_OVERRIDE T_FINAL;

	virtual const TypeInfoSet getProductTypes() const T_OVERRIDE T_FINAL;

	virtual bool isCacheable() const T_OVERRIDE T_FINAL;

	virtual Ref< Object > create(resource::IResourceManager* resourceManager, const TypeInfo& resourceType, const Guid& guid, const Object* current) const T_OVERRIDE T_FINAL;

private:
	Ref< db::Database > m_database;
	Ref< IScriptManager > m_scriptManager;
};

	}
}

#endif	// traktor_script_ScriptModuleFactory_H
