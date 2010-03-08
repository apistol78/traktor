#ifndef traktor_world_EntityInstance_H
#define traktor_world_EntityInstance_H

#include "Core/Guid.h"
#include "Core/RefArray.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
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
class T_DLLCLASS EntityInstance : public ISerializable
{
	T_RTTI_CLASS;

public:
	EntityInstance();

	EntityInstance(const Guid& guid, const std::wstring& name, EntityData* entityData);

	/*! \brief Set guid of instance.
	 *
	 * \param guid New instance guid.
	 */
	virtual void setGuid(const Guid& guid);

	/*! \brief Get guid of instance.
	 *
	 * \param guid Instance guid.
	 */
	virtual const Guid& getGuid() const;

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
	virtual Ref< EntityData > getEntityData() const;

	/*! \brief Set per-instance custom data.
	 *
	 * \param customData Per-instance custom data.
	 */
	virtual void setInstanceData(ISerializable* instanceData);

	/*! \brief Get per-instance custom data.
	 *
	 * \return Instance custom data.
	 */
	virtual Ref< ISerializable > getInstanceData() const;

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
	virtual const std::vector< EntityInstance* >& getReferences() const;

	virtual bool serialize(ISerializer& s);

private:
	Guid m_guid;
	std::wstring m_name;
	Ref< EntityData > m_entityData;
	Ref< ISerializable > m_instanceData;
	std::vector< EntityInstance* > m_references;
};

	}
}

#endif	// traktor_world_EntityInstance_H
