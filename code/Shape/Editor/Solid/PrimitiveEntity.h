#pragma once

#include "Core/Ref.h"
#include "Shape/Editor/Solid/SolidTypes.h"
#include "World/Entity.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SHAPE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace model
	{

class Model;

	}

    namespace shape
    {

/*! Primitive entity.
 * \ingroup Shape
 */
class T_DLLCLASS PrimitiveEntity : public world::Entity
{
    T_RTTI_CLASS;

public:
    PrimitiveEntity(const Transform& transform, BooleanOperation operation);

    virtual void setTransform(const Transform& transform) override final;

    virtual bool getTransform(Transform& outTransform) const override final;

	virtual Aabb3 getBoundingBox() const override final;

    virtual void update(const world::UpdateParams& update) override final;

    const Transform& getTransform() const { return m_transform; }

    BooleanOperation getOperation() const { return m_operation; }

    const model::Model* getModel() const { return m_model; }

    bool isDirty() const { return m_dirty; }

    void resetDirty() { m_dirty = false; }

protected:
    friend class PrimitiveEntityData;

    Transform m_transform;
    BooleanOperation m_operation;
	Ref< const model::Model > m_model;
	Aabb3 m_boundingBox;
    bool m_dirty;
};

    }
}