#ifndef traktor_animation_AnimationFactory_H
#define traktor_animation_AnimationFactory_H

#include "Core/Heap/Ref.h"
#include "Resource/ResourceFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
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
class T_DLLCLASS AnimationFactory : public resource::ResourceFactory
{
	T_RTTI_CLASS(AnimationFactory)

public:
	AnimationFactory(db::Database* db);

	virtual const TypeSet getResourceTypes() const;

	virtual Object* create(const Type& resourceType, const Guid& guid, bool& outCacheable);

private:
	Ref< db::Database > m_db;
};

	}
}

#endif	// traktor_animation_AnimationFactory_H
