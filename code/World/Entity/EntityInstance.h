#ifndef traktor_world_EntityInstance_H
#define traktor_world_EntityInstance_H

#include "Core/Heap/Ref.h"
#include "Core/Serialization/Serializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

class EntityData;

/*! \brief Entity instance description.
 * \ingroup World
 *
 * This class is used by serialization to
 * resolve inter-entity dependencies.
 */
class T_DLLCLASS EntityInstance : public Serializable
{
	T_RTTI_CLASS(EntityInstance)

public:
	EntityInstance();

	EntityInstance(const std::wstring& name, EntityData* entityData);

	virtual void setName(const std::wstring& name);

	virtual const std::wstring& getName() const;

	virtual EntityData* getEntityData() const;

	virtual void addReference(EntityInstance* reference);

	virtual const RefArray< EntityInstance >& getReferences() const;

	virtual bool serialize(Serializer& s);

private:
	std::wstring m_name;
	Ref< EntityData > m_entityData;
	RefArray< EntityInstance > m_references;
};

	}
}

#endif	// traktor_world_EntityInstance_H
