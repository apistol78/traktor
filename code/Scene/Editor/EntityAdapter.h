#ifndef traktor_scene_EntityAdapter_H
#define traktor_scene_EntityAdapter_H

#include "Core/Heap/Ref.h"
#include "Core/Object.h"
#include "Core/Guid.h"
#include "Core/Math/Aabb.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Misc/MD5.h"

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

class EntityEditor;

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

	void setTransform(const Matrix44& transform);

	Matrix44 getTransform() const;

	Aabb getBoundingBox() const;

	//@}

	/*! \name External entity accessors. */
	//@{

	bool isExternal() const;

	bool isChildOfExternal() const;

	bool getExternalGuid(Guid& outGuid) const;

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

	void setEntityEditor(EntityEditor* entityEditor);

	EntityEditor* getEntityEditor() const;

	//@}

	/*! \name Entity state. */
	//@{

	bool isSelected() const;

	void setHash(const MD5& hash);

	const MD5& getHash() const;

	//@}

	/*! \name User object. */
	//@{

	void setUserObject(Object* userObject);

	Object* getUserObject() const;

	template < typename UserType >
	UserType* getUserObject() const
	{
		return dynamic_type_cast< UserType* >(getUserObject());
	}

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

	Ref< world::EntityInstance > m_instance;
	MD5 m_hash;
	Ref< world::Entity > m_entity;
	Ref< EntityAdapter > m_parent;
	RefArray< EntityAdapter > m_children;
	Ref< EntityEditor > m_entityEditor;
	bool m_selected;
	Ref< Object > m_userObject;
};

	}
}

#endif	// traktor_scene_EntityAdapter_H
