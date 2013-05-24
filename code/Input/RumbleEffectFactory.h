#ifndef traktor_input_RumbleEffectFactory_H
#define traktor_input_RumbleEffectFactory_H

#include "Resource/IResourceFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_EXPORT)
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

	namespace input
	{

/*! \brief Rumble effect resource factory.
 * \ingroup Input
 */
class T_DLLCLASS RumbleEffectFactory : public resource::IResourceFactory
{
	T_RTTI_CLASS;

public:
	RumbleEffectFactory(db::Database* db);

	virtual const TypeInfoSet getResourceTypes() const;

	virtual const TypeInfoSet getProductTypes() const;

	virtual bool isCacheable() const;

	virtual Ref< Object > create(resource::IResourceManager* resourceManager, const TypeInfo& resourceType, const Guid& guid, const Object* current) const;

private:
	Ref< db::Database > m_db;
};

	}
}

#endif	// traktor_input_RumbleEffectFactory_H
