#ifndef traktor_input_RumbleEffectFactory_H
#define traktor_input_RumbleEffectFactory_H

#include "Core/Heap/Ref.h"
#include "Resource/ResourceFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_EXPORT)
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

	namespace input
	{

/*! \brief Rumble effect resource factory.
 * \ingroup Input
 */
class T_DLLCLASS RumbleEffectFactory : public resource::ResourceFactory
{
	T_RTTI_CLASS(RumbleEffectFactory)

public:
	RumbleEffectFactory(db::Database* db);

	virtual const TypeSet getResourceTypes() const;

	virtual Object* create(const Type& resourceType, const Guid& guid, bool& outCacheable);

private:
	Ref< db::Database > m_db;
};

	}
}

#endif	// traktor_input_RumbleEffectFactory_H
