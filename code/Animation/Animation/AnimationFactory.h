#ifndef traktor_animation_AnimationFactory_H
#define traktor_animation_AnimationFactory_H

#include "Resource/IResourceFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
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

	namespace animation
	{

/*! \brief Key framed animation factory.
 * \ingroup Animation
 */
class T_DLLCLASS AnimationFactory : public resource::IResourceFactory
{
	T_RTTI_CLASS;

public:
	AnimationFactory(db::Database* db);

	virtual const TypeInfoSet getResourceTypes() const;

	virtual const TypeInfoSet getProductTypes() const;

	virtual bool isCacheable() const;

	virtual Ref< Object > create(resource::IResourceManager* resourceManager, const TypeInfo& resourceType, const Guid& guid) const;

private:
	Ref< db::Database > m_db;
};

	}
}

#endif	// traktor_animation_AnimationFactory_H
