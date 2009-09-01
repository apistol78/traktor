#ifndef traktor_scene_EntityAdapter_H
#define traktor_scene_EntityAdapter_H

#include "Core/Heap/Ref.h"
#include "Core/Object.h"
#include "Core/Guid.h"
#include "Core/Math/Aabb.h"
#include "Core/Containers/AlignedVector.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

class EntityInstance;
class EntityData;
class Entity;

	}

	namespace scene
	{

class IEntityEditor;

/*! \brief Entity adapter class.
 *
 * Map between EntityInstance and Entity.
 * Also keep parent-child relationship of entities.
 */
class T_DLLCLASS EntityAdapter : public Object
{
	T_RTTI_CLASS(EntityAdapter)

public:
	EntityAdapter(world::EntityInstance* instance);

	/*! \name Accessors */
	//@{

	world::EntityInstance* getInstance() const;

	world::EntityData* getEntityData() const;

	void setRealEntityData(world::EntityData* entityData);

	world::EntityData* getRealEntityData() const;

	void setEntity(world::Entity* entity);

	world::Entity* getEntity() const;

	//@}

	/*! \name Information accessors. */
	//@{

	std::wstring getName() const;

	std::wstring getTypeName() const;

	//@}

	/*! \name Spatial entity accessors. */
	//@{

	bool isSpatial() const;

	void setTransform(const Transform& transform);

	Transform getTransform() const;

	Aabb getBoundingBox() const;

	//@}

	/*! \name External entity accessors. */
	//@{

	bool isExternal() const;

	bool isChildOfExternal() const;

	bool getExternalGuid(Guid& outGuid) const;

	//@}

	/*! \name References. */
	//@{

	bool addReference(EntityAdapter* reference);

	void removeReference(EntityAdapter* reference);

	//@}

	/*! \name Relationship. */
	//@{

	bool isGroup() const;

	EntityAdapter* getParent() const;

	EntityAdapter* getParentGroup();

	EntityAdapter* getParentContainerGroup();

	bool addChild(EntityAdapter* child, bool modifyEntityData);

	void removeChild(EntityAdapter* child, bool modifyEntityData);

	const RefArray< EntityAdapter >& getChildren() const;

	void unlink();

	//@}

	/*! \name Entity editor. */
	//@{

	void setEntityEditor(IEntityEditor* entityEditor);

	IEntityEditor* getEntityEditor() const;

	//@}

	/*! \name Entity state. */
	//@{

	bool isSelected() const;

	//@}

	/*! \name Editor aid. */
	//@{

	struct SnapPoint
	{
		Vector4 position;
		Vector4 direction;
	};

	AlignedVector< SnapPoint > getSnapPoints() const;

	//@}

private:
	friend class SceneEditorContext;

	Ref< world::EntityData > m_realEntityData;
	Ref< world::EntityInstance > m_instance;
	Ref< world::Entity > m_entity;
	Ref< EntityAdapter > m_parent;
	RefArray< EntityAdapter > m_children;
	Ref< IEntityEditor > m_entityEditor;
	bool m_selected;
};

	}
}

#endif	// traktor_scene_EntityAdapter_H
