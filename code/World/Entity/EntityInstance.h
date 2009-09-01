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
 *
 * The entity product of this instance will
 * have references to those entities produced
 * by the instance references.
 */
class T_DLLCLASS EntityInstance : public Serializable
{
	T_RTTI_CLASS(EntityInstance)

public:
	EntityInstance();

	EntityInstance(const std::wstring& name, EntityData* entityData);

	/*! \brief Set name of instance.
	 *
	 * \param name New instance name.
	 */
	virtual void setName(const std::wstring& name);

	/*! \brief Get name of instance.
	 *
	 * \return Instance name.
	 */
	virtual const std::wstring& getName() const;

	/*! \brief Set instance entity data.
	 *
	 * \param entityData New instance entity data.
	 */
	virtual void setEntityData(EntityData* entityData);

	/*! \brief Get instance entity data.
	 *
	 * \return Instance entity data.
	 */
	virtual EntityData* getEntityData() const;

	/*! \brief Set per-instance custom data.
	 *
	 * \param customData Per-instance custom data.
	 */
	virtual void setInstanceData(Serializable* instanceData);

	/*! \brief Get per-instance custom data.
	 *
	 * \return Instance custom data.
	 */
	virtual Serializable* getInstanceData() const;

	/*! \brief Add instance reference.
	 *
	 * \param reference Reference to add.
	 */
	virtual void addReference(EntityInstance* reference);

	/*! \brief Remove instance reference.
	 *
	 * \param reference Reference to remove.
	 */
	virtual void removeReference(EntityInstance* reference);

	/*! \brief Remove all instance references. */
	virtual void removeAllReferences();

	/*! \brief Get all instance references.
	 *
	 * \return Array of instance's references.
	 */
	virtual const RefArray< EntityInstance >& getReferences() const;

	virtual int getVersion() const;

	virtual bool serialize(Serializer& s);

private:
	std::wstring m_name;
	Ref< EntityData > m_entityData;
	Ref< Serializable > m_instanceData;
	RefArray< EntityInstance > m_references;
};

	}
}

#endif	// traktor_world_EntityInstance_H
